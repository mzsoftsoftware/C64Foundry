# IEC Subsystem Architecture

## Purpose

This document records the agreed target architecture and the current implementation boundary of the C64Foundry IEC subsystem.

The IEC subsystem provides communication between the emulated C64 and configured IEC peripherals, including the ASCII printer and the Foundry Drive. The current storage target is a practical D64-based implementation of standard IEC and drive/DOS behavior. Emulation of the internal 1541 CPU and physical drive electronics is not required by the current compatibility target.

The architecture separates electrical bus handling, IEC protocol handling, generic device routing, drive command logic, physical/logical disk objects and disk-image storage.

## Components and responsibilities

### `IECBus`

- Represents the electrical IEC bus and its signal lines.
- Provides the bus-level operations needed to read and drive IEC signals.
- Does not know about IEC devices, device types, device addresses or protocol commands.
- Does not dispatch protocol events to devices.

### `IECProtocol`

- There is exactly one central protocol instance in `IECSystem`.
- Uses the bus to observe and drive IEC signals.
- Implements IEC-level timing and protocol state.
- Interprets bus activity and emits typed, device-neutral protocol events.
- Accepts protocol responses from the device manager and performs the corresponding bus transfer.
- Knows IEC protocol state and the currently addressed device number, but does not know concrete devices or hold device pointers.
- Does not decide which concrete device receives an event.

The protocol events preserve the information needed by devices, including the IEC device number, communication mode, secondary address, received byte and EOI state as applicable. LISTEN, UNLISTEN, TALK, UNTALK, SECONDARY ADDRESS, CLOSE and OPEN must remain distinguishable in the event model.

Relevant ATN command ranges:

| Command | Values |
|---|---|
| LISTEN | `$20`–`$3E` |
| UNLISTEN | `$3F` |
| TALK | `$40`–`$5E` |
| UNTALK | `$5F` |
| SECONDARY ADDRESS | `$60`–`$6F` |
| CLOSE | `$E0`–`$EF` |
| OPEN | `$F0`–`$FF` |

### `IECDeviceManager`

The manager performs generic address registration and protocol-event routing.

- Stores non-owning references to generic `IECDevice` objects.
- Maps each registered IEC address to one device.
- A device may register more than one IEC address.
- Dispatches protocol events to the device registered for the event's IEC address.
- Routes the device's `IECProtocolResponse` back to the central protocol.
- Rejects invalid or duplicate address registrations without leaving a partial mapping.
- Removes all address mappings belonging to a device when it is unregistered.
- Does not create, own or destroy devices.
- Does not know concrete device classes or their internal structure.
- Does not know about disks, disk images, printer jobs or DOS commands.

The address domain used by the Foundry Drive is #8 through #11. It is a small bounded domain; fixed-size lookup is preferred over a hash map in the hot path where practical.

### `IECDevice`

`IECDevice` is the generic interface for a peripheral visible on the IEC bus.

- Receives typed protocol events through the manager.
- Produces a protocol response where a response is required.
- Reports the IEC device numbers it currently supports.
- Exposes device-originated GUI/status events through generic event types.
- Does not access `IECBus` or `IECProtocol` directly.
- Does not own or instantiate a per-device protocol instance.

`DeviceId` identifies the configured device object. It is not an IEC device number. One `DeviceId` may therefore correspond to multiple IEC addresses.

### `IECDeviceFactory`

- Creates and owns concrete IEC device objects.
- Applies the device configuration supplied by `IECSystem`.
- Provides the lifetime boundary for shared backend resources used by concrete devices.
- Unregisters devices through the system/manager coordination before destroying them.
- Does not implement IEC signaling or protocol timing.
- Does not participate in the per-cycle hot path.

The factory is owned by `EmulatorWorker`. Factory operations and device lifecycle changes execute in the emulation thread.

### `IECSystem`

`IECSystem` owns the central IEC infrastructure:

- `IECBus`
- one `IECProtocol`
- `IECDeviceManager`

It connects the bus to the C64 machine, coordinates protocol/manager event flow, handles IEC reset and coordinates configured device lifecycle with the factory. It does not own concrete devices or a Foundry Drive.

## Device ownership: Foundry Drive and disks

### `IECFoundryDrive`

`IECFoundryDrive` is one generic `IECDevice`, not a separate IEC device for every disk slot.

The drive:

- Implements IEC-facing drive behavior and the commands sent by the C64, including directory and file operations such as `LOAD "$"` and, as implemented later, the broader DOS command set.
- Manages secondary addresses and channels.
- Exposes any configured IEC device numbers in the range #8 through #11.
- Selects the internal disk corresponding to the IEC number addressed by the C64.
- Owns and manages its `IECFoundryDisk` objects.
- Delegates media and sector operations to the selected disk.

The available numbers #8, #9, #10 and #11 are independently configurable. Any subset may be enabled; for example, a configuration containing only #9 is valid. A single `IECFoundryDrive` object is registered with the manager under only the numbers it currently exposes.

### `IECFoundryDisk`

`IECFoundryDisk` is an internal drive component, not an `IECDevice`.

- It has no IEC device number and does not participate in manager registration.
- It represents one drive/media slot managed by `IECFoundryDrive`.
- It manages one `DiskImage`.
- It provides the drive with track/sector access, including reading and writing sector data.
- It owns or controls the lifecycle of its attached image according to the disk-image API.
- It can be exchanged with another disk object without changing the IEC address exposed by the drive.

The distinction is intentional: the drive handles IEC commands and channel semantics; the disk handles the mounted medium and its track/sector data.

### `DiskImage`

`DiskImage` represents the actual image data and image-format operations. It does not know an IEC device number. `IECFoundryDisk` uses it to provide media access to the drive.

This separation leaves room for future storage backends and media types, including HDD-like storage or other non-D64 media, without teaching `IECDeviceManager` or `IECProtocol` about them.

### Fixed disk-slot array

The Foundry Drive has four possible slots corresponding to IEC numbers #8 through #11. The drive uses a fixed-size array of disk pointers:

```cpp
static constexpr quint8 FIRST_DEVICE_NUMBER = 8;
static constexpr quint8 LAST_DEVICE_NUMBER = 11;
static constexpr quint8 DEVICE_COUNT = LAST_DEVICE_NUMBER - FIRST_DEVICE_NUMBER + 1;

IECFoundryDisk* m_disks[DEVICE_COUNT] = {};
```

- Array index 0 corresponds to IEC #8; index 3 corresponds to IEC #11.
- A non-null pointer means that the slot has a configured disk.
- `nullptr` means that the slot is unused.
- IEC number to array index conversion is `deviceNumber - FIRST_DEVICE_NUMBER`, after range validation.
- The drive reports only the addresses whose array entries are populated.
- The array is a fixed, bounded structure; no hash lookup or dynamic slot container is needed.

The IEC number belongs to the drive's slot mapping, never to `IECFoundryDisk`.

A disk swap changes the disk pointer associated with a slot. The slot's IEC number remains unchanged.

## Event flow

### C64 to device

1. C64-side electrical activity changes the bus.
2. `IECProtocol` observes the bus and advances protocol state.
3. `IECProtocol` emits a typed protocol event.
4. `IECDeviceManager` routes the event using its IEC-address mapping.
5. The matching device processes the event and fills an `IECProtocolResponse`.
6. The manager returns the response to `IECProtocol`.
7. `IECProtocol` performs the required IEC transfer through the bus.

### Device to C64

Devices do not bypass the manager to access the protocol or bus. A device response is returned through the manager to the central protocol, which owns the electrical transfer and handshake.

## Registration and lifetime

- The factory owns every concrete device it creates.
- The manager stores non-owning device references.
- Registration supplies one device and all IEC addresses currently exposed by that device.
- Registration is atomic: invalid, duplicate or conflicting addresses must not leave a partial mapping.
- Unregistration removes every address belonging to that device and clears selection state referring to it.
- A device must be unregistered before the factory destroys it.
- Device creation, configuration, registration and destruction happen in the emulation thread.
- No registration or destruction occurs in the normal emulation-cycle path.

`IECFoundryDrive` is registered once, with the subset of #8–#11 represented by populated disk slots. The manager maps each such address to the same drive pointer.

## Events to the GUI

Device events use generic event types defined in the IEC core. Examples include printer output events and drive sector-access status events. A drive access event carries the relevant IEC number together with the access operation and track/sector/error information; the GUI does not need a concrete device pointer.

`IECSystem` collects and exposes device events. `EmulatorWorker` forwards the event batches through its existing controller-facing signals. Printer output revision tracking is not part of the new event path.

## Timing and performance

- One central `IECProtocol`; no per-device protocol instances.
- Direct event dispatch and bounded address lookup.
- No factory work, registration or destruction in the normal emulation-cycle path.
- No per-cycle temporary containers or avoidable dynamic allocations.
- Avoid unnecessary `QByteArray` copies and verbose logging in timing-sensitive paths.
- Internal synchronous calls remain direct calls, not Qt signal/slot traffic on every cycle.
- Each time-sensitive component is advanced exactly once.
- Preserve existing electrical handshake behavior unless a separate protocol change is explicitly agreed.

## Current implementation boundary

The controller, worker and scheduler ownership refactor has been completed and committed. `IECSystem` has been introduced and committed as the central IEC runtime boundary. The generic device, protocol-event, response, manager and factory headers have been aligned with the agreed architecture.

The manager implementation and migration of the central protocol and concrete devices remain part of the ongoing IEC refactor. This document describes the agreed architecture, not a claim that all of it is already implemented.

## Explicit non-goals

- Emulating the physical 1541 CPU, VIA chips, GCR encoding, rotational timing or copy protection as part of the current DOS target.
- Giving `IECDeviceManager` knowledge of concrete drive, disk or printer types.
- Registering `IECFoundryDisk` as an IEC device.
- Giving `IECFoundryDisk` an IEC device number.
- Moving disk/image ownership into `IECDeviceManager` or `IECSystem`.
- Creating devices from inside `IECDeviceManager`.
