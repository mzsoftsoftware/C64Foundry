# IEC Subsystem Architecture

## Purpose

This document records the agreed target architecture for the C64Foundry IEC subsystem.

The IEC subsystem provides communication between the emulated C64 and configured IEC peripherals, including the ASCII printer and the Foundry Disk drive.

The target is a practical D64-based implementation of standard IEC and drive/DOS behavior. It is not intended to emulate the internal 1541 CPU and physical drive electronics.

## Components and responsibilities

### `IECBus`

- Represents the electrical IEC bus and its signal lines.
- Provides the bus-level operations needed to read and drive IEC signals.
- Does not know about IEC devices, device types, device addresses or protocol commands.
- Does not dispatch protocol events to devices.

### `IECProtocol`

- Uses the bus to observe and drive IEC signals.
- Implements IEC-level timing and protocol state.
- Interprets bus activity and emits typed protocol events.
- Accepts outgoing protocol operations requested through the device manager.
- Does not know concrete devices or hold device pointers.
- Does not decide which device should receive an event.

The protocol must distinguish command classes rather than collapsing them into a generic address event. In particular, LISTEN, UNLISTEN, TALK, UNTALK, SECONDARY ADDRESS, CLOSE and OPEN must remain distinguishable.

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

The protocol must preserve the IEC address and, where applicable, the secondary address/channel in the emitted event.

### `IECDeviceManager`

- Knows registered generic `IECDevice` objects and their IEC addresses.
- Maps an IEC address to at most one device.
- Dispatches protocol events to the device or devices selected by the event and current IEC selection state.
- Routes device-originated transmit requests back to `IECProtocol`.
- Does not create, own or destroy devices.
- Does not know concrete device classes or their internal structure.
- Does not know that a device is a printer, drive or disk implementation.
- Ignores/unroutes events for unregistered addresses.

The manager's central mapping is conceptually:

`IEC address -> IECDevice`

The address domain is small and bounded (0–15). A fixed-size lookup table is preferred over a hash map for the hot path, provided the final interface and address validation support it.

A single device may register multiple IEC addresses. A given IEC address may be registered by only one device at a time.

The manager must support multiple selected LISTEN devices where the protocol semantics require it; it must not assume that only one listener can be active.

### `IECDevice`

- Represents a generic IEC peripheral.
- Receives typed IEC protocol events through the manager.
- Requests outgoing IEC transfers through the manager.
- Does not access `IECBus` directly.
- Does not own or instantiate the bus or protocol.
- Does not create a per-device `IECProtocol` instance.

The final public interface is to be agreed before implementation. Event payloads should contain only meaningful fields and should not overload one field with unrelated command meanings.

### `IECDeviceFactory`

- Creates and owns concrete IEC device objects.
- Applies the device configuration supplied by the application/GUI.
- Registers each created device and its IEC addresses with `IECDeviceManager`.
- Unregisters devices before destroying them.
- Is the sole owner responsible for the lifetime of the concrete IEC device objects.
- Does not implement IEC signaling or protocol timing.
- Does not participate in the per-cycle hot path.

The factory is owned by `EmulatorWorker`. It must perform creation, registration, removal and destruction in the emulation thread.

The factory must not be called by the manager. The dependency is one-way:

`IECDeviceFactory -> IECDeviceManager`

### `IECSystem`

`IECSystem` groups and owns the IEC infrastructure:

- `IECBus`
- `IECProtocol`
- `IECDeviceManager`

It creates and connects these components and provides the narrow access points needed by the machine, worker and factory. It coordinates IEC initialization and reset as appropriate.

`IECSystem` does not create or own concrete peripheral devices. That responsibility belongs to `IECDeviceFactory`.

## Device-specific ownership

### Foundry Drive

`IECFoundryDrive` is one generic IEC device that can expose multiple IEC addresses, for example 8, 9, 10 and 11.

- The drive manages its own configured IEC addresses.
- The drive owns and manages its internal disk/image state.
- The drive reports the IEC addresses it exposes when it is registered.
- The manager maps each of those addresses to the same drive object.
- The manager does not know about the drive's internal disks.

The drive is not four separate manager-owned devices merely because it exposes four IEC addresses.

### Internal disk objects

Disk objects remain owned and managed by `IECFoundryDrive`. They are internal drive components, not independently registered IEC devices in the target architecture.

### ASCII printer

`IECAsciiPrinter` remains a concrete IEC device. It communicates through the manager and does not own a separate protocol instance or access the bus directly.

## Event flow

### C64 to device

1. C64-side electrical activity changes the bus.
2. `IECProtocol` observes the bus and processes IEC timing/state.
3. `IECProtocol` emits a typed protocol event.
4. `IECDeviceManager` evaluates the event and current selection state.
5. The manager dispatches the event to the matching registered device or devices.
6. An unknown/unregistered address is not dispatched.

### Device to C64

1. A device requests transmission through `IECDeviceManager`.
2. The manager validates/routes the request to `IECProtocol`.
3. `IECProtocol` performs the IEC handshake and drives the bus.
4. The bus exposes the resulting electrical signal changes to the C64.

Devices never bypass the manager to access the protocol or bus.

## Registration and lifetime

- The factory owns every concrete device it creates.
- The manager stores non-owning device references.
- Registration includes the device and all IEC addresses it exposes.
- Registration must reject invalid addresses, duplicate addresses and conflicting registrations without leaving a partial mapping.
- Unregistration removes every address belonging to that device and clears any manager selection state that refers to it.
- The factory unregisters before destruction.
- Device creation and destruction are performed in the emulation thread.
- No registration or destruction occurs in the normal emulation-cycle path.

The exact signatures and error-reporting mechanism are deliberately left for the interface-design step.

## Timing and performance

- There is one central `IECProtocol`, not one protocol instance per device.
- The protocol owns no device list.
- The manager performs direct dispatch and address lookup.
- The manager and protocol are advanced through one clearly defined runtime path.
- A device must not be clocked twice by both the manager and scheduler.
- No per-cycle temporary containers, dynamic allocations, avoidable `QByteArray` copies or verbose logging.
- Device registration and configuration happen outside normal IEC transfer processing.
- Keep the existing electrical handshake behavior intact during the architectural refactor unless a separate protocol bug is being addressed.

## Migration order

1. Agree the public interfaces for `IECDevice`, protocol events, `IECDeviceManager`, `IECSystem` and `IECDeviceFactory`.
2. Refactor `EmulatorController` and `EmulatorWorker` ownership/initialization.
3. Introduce `IECSystem` and the factory wiring.
4. Implement the generic `IECDeviceManager`.
5. Adapt `IECBus` and `IECProtocol` to the new event and request paths.
6. Adapt `IECAsciiPrinter` and `IECFoundryDrive`.
7. Keep internal disk/image management inside the drive.
8. Verify scheduler integration so the protocol and devices are advanced exactly once.

## Explicit non-goals for this refactor

- Rewriting IEC electrical timing without evidence that it is necessary.
- Adding physical 1541 CPU/VIA/GCR emulation.
- Moving disk ownership out of `IECFoundryDrive`.
- Teaching `IECDeviceManager` about concrete peripheral types.
- Creating devices from inside `IECDeviceManager`.
