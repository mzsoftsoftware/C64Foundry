
# IEC Subsystem Architecture

## Purpose

This document describes the architecture and current implementation of the C64Foundry IEC subsystem.

The IEC subsystem provides communication between the emulated C64 and configured IEC peripherals, including the ASCII printer and the Foundry Drive.

The current storage target is a practical D64-based implementation of standard IEC and drive/DOS behavior. Emulation of the internal 1541 CPU and physical drive electronics is not required by the current compatibility target.

The architecture separates:

- Electrical bus handling
- IEC protocol handling
- Generic device routing
- Device creation and ownership
- Drive and disk management
- Device-originated events and GUI communication

The architecture is designed so that concrete devices do not need direct access to the IEC bus or protocol implementation.

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

Protocol events preserve the information needed by devices, including:

- IEC device number
- Communication mode
- Secondary address
- Received byte
- EOI state, where applicable

The following ATN commands remain distinguishable in the event model:

| Command | Values | Event |
|---|---|---|
| LISTEN | `$20`–`$3E` | `DeviceSelected` with Listen mode |
| UNLISTEN | `$3F` | `EndOfTransmission` and `DeviceDeselected` |
| TALK | `$40`–`$5E` | `DeviceSelected` with Talk mode |
| UNTALK | `$5F` | `DeviceDeselected` |
| SECONDARY ADDRESS | `$60`–`$6F` | `SecondaryAddressReceived` |
| CLOSE | `$E0`–`$EF` | `Close` |
| OPEN | `$F0`–`$FF` | `Open` |

The lower four bits of SECONDARY ADDRESS, CLOSE and OPEN are preserved as the secondary address.

The protocol owns all electrical handshake and timing behavior. Concrete devices must not implement their own bus-level protocol handling.

### `IECDeviceManager`

The manager performs generic address registration and protocol-event routing.

- Stores non-owning references to generic `IECDevice` objects.
- Maps each registered IEC address to one device.
- A device may register more than one IEC address.
- Dispatches protocol events to the device registered for the event's IEC address.
- Routes the device's `IECProtocolResponse` back to the central protocol.
- Rejects invalid or duplicate address registrations.
- Does not create, own or destroy devices.
- Does not know concrete device classes or their internal structure.
- Does not know about disks, disk images, printer jobs or DOS commands.

The manager supports IEC addresses #0 through #30.

Address registration is independent of device type and does not imply that a device has a separate IEC address of its own. A device may reserve and register a range of addresses.

Registration and unregistration are performed per address. `IECSystem` coordinates the complete registration or unregistration of a device, including all addresses reserved by that device.

The manager deduplicates device pointers when resetting devices or collecting their events. This is necessary because one device may be registered under several IEC addresses.

### `IECDevice`

`IECDevice` is the generic interface for a peripheral visible on the IEC bus.

- Receives typed protocol events through the manager.
- Produces a protocol response where a response is required.
- Can reset its internal state.
- Exposes device-originated events through generic event types.
- Does not access `IECBus` or `IECProtocol` directly.
- Does not own or instantiate a per-device protocol instance.

`DeviceId` identifies the configured device object. It is not an IEC device number.

One `DeviceId` may therefore correspond to multiple IEC addresses.

### `IECDeviceFactory`

- Creates and owns concrete IEC device objects.
- Applies the device configuration supplied by `IECSystem`.
- Provides the lifetime boundary for concrete devices.
- Destroys devices when requested by `IECSystem`.
- Does not implement IEC signaling or protocol timing.
- Does not participate in the normal per-cycle protocol path.

The factory is owned by `EmulatorWorker`. Factory operations and device lifecycle changes execute in the emulation thread.

The factory also provides the available device types and the supported IEC addresses for each device type. `IECSystem` exposes this information to the application without requiring the GUI to access the factory directly.

### `IECSystem`

`IECSystem` is the central runtime boundary of the IEC subsystem.

It owns:

- `IECBus`
- One `IECProtocol`
- One `IECDeviceManager`

It references:

- The `C64Machine`
- The `IECDeviceFactory`

`IECSystem` is responsible for:

- Connecting the IEC bus to the C64 machine.
- Advancing the central IEC protocol.
- Routing protocol events through the manager.
- Returning device responses to the protocol.
- Managing configured devices and their lifecycle.
- Validating device configurations and address reservations.
- Registering and unregistering device addresses.
- Handling IEC reset.
- Collecting system and device events.
- Processing generic system and device requests.

`IECSystem` does not own concrete devices. Their ownership belongs to `IECDeviceFactory`.

`IECSystem` does not know about `IECFoundryDisk` or individual disk-image operations.

### `EmulatorWorker` and `EmulationScheduler`

`EmulatorWorker` owns the runtime components required by the emulation thread:

- `C64Machine`
- `IECDeviceFactory`
- `IECSystem`
- `EmulationScheduler`

The worker acts as the GUI-independent mediator between the application and the emulation runtime.

`EmulationScheduler` advances the C64 machine and IEC system in a defined order. Each emulation cycle advances the machine once and the IEC system once.

The worker forwards requests to the appropriate runtime component and forwards system and device events to the application through its existing controller-facing interface.

The controller does not expose the machine directly.

## Device ownership: Foundry Drive and disks

### `IECFoundryDrive`

`IECFoundryDrive` is one generic `IECDevice`. It represents one logical drive system and may expose multiple IEC addresses.

A drive has its own internal `DeviceId`, but it does not have an IEC device number of its own.

The drive:

- Owns and manages its `IECFoundryDisk` objects.
- Manages a configurable number of disk slots.
- Reserves a contiguous IEC address range for its slots.
- Maps an addressed IEC number to the corresponding slot.
- Provides operations to enable or disable a disk.
- Provides operations to insert, eject and save an image.
- Collects image and sector-access events from its disks.
- Reports these events through the generic device-event interface.

The drive is the intended location for IEC-facing drive behavior, including secondary-address and channel management, directory and file operations, and DOS command handling.

These IEC drive operations are not yet implemented. In particular, `IECFoundryDrive::processEvent()` currently does not implement drive command processing. They belong to the separate FoundryDrive implementation work package and are not a prerequisite for the completed IEC architecture refactoring.

Multiple independent `IECFoundryDrive` instances are supported. Each drive has its own configuration, identity, disk slots and reserved IEC address range.

### Foundry Drive configuration

`IECFoundryDriveConfiguration` contains:

- The start IEC device number.
- The configured disk-slot configurations.

The number of configured slots is available through `slotCount()`.

The end address is derived:

`endDeviceNumber = startDeviceNumber + slotCount - 1`

The end address is not stored independently.

The current maximum number of disk slots per drive is four. This is a current configuration limit, not a fundamental architectural limitation.

Each `IECFoundryDiskConfiguration` contains:

- Disk ID
- Slot index
- IEC device number
- Enabled state
- Image file name
- `saveCurrentDiskOnExit`

The complete address range is reserved by the drive, including addresses whose corresponding disk slot does not currently contain a disk object.

The range must not overlap with another drive or any other IEC device.

The same `IECFoundryDrive` instance is registered with the manager under every address in its reserved range.

The drive uses the addressed IEC number to determine the corresponding slot.

### Configuration changes

The drive applies configuration changes according to the distinction between structural changes and disk-level changes.

Changes to the start IEC address or slot count rebuild the affected disk objects.

Changes to disk-level settings, including the enabled state and image assignment, are applied to the existing disk objects where possible.

This behavior is intentional. The drive configuration is the authoritative configuration for the drive and its disk slots.

### `IECFoundryDisk`

`IECFoundryDisk` is an internal drive component, not an `IECDevice`.

- It has no independent IEC device registration.
- It is owned by `IECFoundryDrive`.
- It represents one configured disk slot.
- It manages its enabled state and image configuration.
- It stores the current image data in memory.
- It provides track/sector access to the image data.
- It supports directory-program generation and PRG-file reading.
- It collects image and sector-access status events for the drive.
- It handles image persistence according to `saveCurrentDiskOnExit`.

The disk object does not know about the IEC bus or protocol.

The disk object does not implement IEC command handling, secondary addresses or channels.

Sector writing is not yet implemented. The current disk functionality is therefore not a complete DOS implementation.

### Disk-slot storage

`IECFoundryDrive` currently stores its disk objects in a `QList<IECFoundryDisk*>`.

The list represents the disk objects managed by the drive. Each disk object stores its slot index.

The IEC address assigned to a slot is derived from the drive's start address and the slot index:

`deviceNumber = startDeviceNumber + slotIndex`

The IEC address remains reserved independently of whether a disk image is currently inserted.

The implementation must not assume that the first slot always corresponds to IEC #8 or that a drive can only expose #8 through #11.

The drive owns the disk objects and destroys them when they are removed or when the drive itself is destroyed.

## Event flow

### C64 to device

1. C64-side electrical activity changes the bus.
2. `IECProtocol` observes the bus and advances protocol state.
3. `IECProtocol` emits a typed protocol event.
4. `IECDeviceManager` routes the event using its IEC-address mapping.
5. The matching device processes the event and fills an `IECProtocolResponse`, if required.
6. The manager returns the response to `IECSystem`.
7. `IECSystem` passes the response to `IECProtocol`.
8. `IECProtocol` performs the required IEC transfer through the bus.

### Device to C64

Devices do not bypass the manager to access the protocol or bus.

A device response is returned through the manager to `IECSystem` and then to the central protocol, which owns the electrical transfer and handshake.

### Device to GUI

Concrete devices collect their own events.

`IECDeviceManager` retrieves events from registered devices. A device registered under several addresses is queried only once.

`IECSystem` collects and exposes the device events. `EmulatorWorker` forwards event batches through its controller-facing signals.

Examples include:

- Printer output events
- Drive image events
- Drive sector-access events

Drive access events contain the relevant device ID and IEC device number, operation, track, sector and success/error information.

The GUI does not need a concrete device pointer.

System lifecycle events, such as device addition, removal, configuration changes and configuration restoration, are exposed separately from device-originated events.

## Requests

Requests from outside the IEC runtime are handled through `EmulatorWorker` and `IECSystem`.

The request hierarchy distinguishes between:

- System requests, which operate on `IECSystem`.
- Device requests, which target a configured device by `DeviceId`.

Device-specific requests use the generic device request/response interface and execute against the selected `IECDevice`.

The GUI does not directly call methods on concrete devices.

## Registration and lifetime

- The factory owns every concrete device it creates.
- The manager stores non-owning device references.
- Registration maps every IEC address reserved by a device to that device.
- A drive is registered under every address in its configured range, including addresses whose disk slots are empty.
- The complete address range is validated before registration.
- Invalid, duplicate or conflicting addresses must not leave a partial mapping.
- No two devices may reserve or register the same IEC address.
- `IECSystem` unregisters all addresses belonging to a device before asking the factory to destroy it.
- Device creation, configuration, registration and destruction happen in the emulation thread.
- No registration or destruction occurs in the normal emulation-cycle path.

A Foundry Drive reserves a contiguous IEC address range determined by its configured slot count and start address.

The same drive pointer is registered for every address in that range. The manager does not distinguish between populated and empty disk slots.

The drive itself decides how to respond when the C64 accesses an address whose corresponding disk slot is empty.

## Timing and performance

- One central `IECProtocol`; no per-device protocol instances.
- Direct event dispatch and bounded address lookup.
- No factory work, registration or destruction in the normal emulation-cycle path.
- No per-cycle temporary containers or avoidable dynamic allocations.
- Avoid unnecessary `QByteArray` copies and verbose logging in timing-sensitive paths.
- Internal synchronous calls remain direct calls, not Qt signal/slot traffic on every cycle.
- Each time-sensitive component is advanced exactly once.
- Preserve existing electrical handshake behavior unless a separate protocol change is explicitly agreed.

## Current implementation status

The IEC architecture refactoring has been completed.

The following parts are integrated:

- GUI-independent `EmulatorWorker` ownership of runtime components.
- `EmulationScheduler` for coordinated machine and IEC clocking.
- Central `IECSystem` as the IEC runtime boundary.
- Central `IECBus` and `IECProtocol`.
- Generic protocol events and protocol responses.
- Generic `IECDevice` interface.
- `IECDeviceManager` with multi-address registration and event routing.
- `IECDeviceFactory` with concrete device ownership.
- Device configuration validation and address reservation.
- Generic system and device requests.
- Generic system and device events.
- Distinction between SECONDARY ADDRESS, OPEN and CLOSE protocol events.

The application compiles and starts with the integrated architecture.

The Foundry Drive's actual IEC command handling is not part of the completed refactoring. Directory transfer, PRG loading through IEC, channel handling, DOS status handling, sector writing and further DOS commands remain separate implementation work.

## Explicit non-goals

- Emulating the physical 1541 CPU, VIA chips, GCR encoding, rotational timing or copy protection as part of the current DOS target.
- Giving `IECDeviceManager` knowledge of concrete drive, disk or printer types.
- Registering `IECFoundryDisk` as an IEC device.
- Giving `IECFoundryDisk` an IEC device number.
- Moving disk ownership into `IECDeviceManager` or `IECSystem`.
- Creating devices from inside `IECDeviceManager`.
- Implementing Foundry Drive DOS and file-transfer functionality as part of the IEC architecture refactoring.

