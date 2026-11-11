# Work Package: IEC Architecture Refactoring

**Status:** In progress  
**Last updated:** 2026-11-07  
**Current step:** Implement the generic `IECDeviceManager` against the agreed interfaces

## 1. Goal

Restructure the emulator runtime and IEC subsystem so that:

- The controller is an external request/notification boundary, independent of IEC implementation details.
- The worker owns the runtime components used by the emulation thread.
- The scheduler advances the C64 machine and IEC system exactly once per cycle.
- `IECSystem` owns the central IEC infrastructure.
- One central protocol translates electrical IEC activity into device-neutral events.
- The generic manager routes events to devices by IEC address.
- The factory owns concrete IEC devices and their shared backend resources.
- The Foundry Drive owns its internal disk objects.
- The emulation hot path remains efficient.

Preserve existing printer and disk behavior unless a behavior change is explicitly included in a later implementation step.

## 2. Completed implementation steps

The following steps have been implemented and committed:

### 2.1 `EmulatorController`

- Controller owns/manages the worker and its thread.
- Controller is the external request/notification boundary, not specifically a GUI-only interface.
- Emulator and IEC requests use separate request paths and response signals.
- Controller uses signals/slots to communicate with the worker.
- Controller does not expose `machine()` or own IEC infrastructure.
- Program-load sequencing is the caller's responsibility; the controller forwards requests rather than implementing a multi-step load workflow.

### 2.2 `EmulatorWorker`

- Worker owns `C64Machine`, `IECDeviceFactory`, `IECSystem` and `EmulationScheduler`.
- Worker does not know concrete IEC device classes.
- Worker processes IEC requests through `IECSystem`.
- Worker collects and forwards generic IEC system/device event batches.
- IEC reset is delegated to `IECSystem::reset()`.
- Final machine shutdown is handled by the run-loop shutdown path.

### 2.3 `EmulationScheduler`

The scheduler has the central machine/system interface:

```cpp
explicit EmulationScheduler(C64Machine* ptrMachine, IECSystem* ptrIECSystem);
```

- `clock()` calls `C64Machine::clock()` and then `IECSystem::clock()`.
- `runCycles()` repeatedly calls `clock()`.
- The worker does not separately clock the IEC system.

### 2.4 `IECSystem`

- `IECSystem` is created by `EmulatorWorker`.
- `IECSystem` creates and owns `IECBus`, one central `IECProtocol` and `IECDeviceManager`.
- `IECSystem` does not own or create the factory.
- `IECSystem` does not directly own the Foundry Drive or other concrete devices.
- Printer-output revision tracking was removed/deferred in favor of generic device events.

### 2.5 Generic interface headers

The following interface/header decisions have been implemented and committed:

- `IECDevice` uses the shared `DeviceId` type from `IECDeviceConfiguration`.
- `IECDevice` receives `IECProtocolEvent` and returns an `IECProtocolResponse`.
- `IECProtocolEvent.h` defines the common protocol event and response types.
- `IECProtocolResponse` distinguishes `None` and `SendData`; its payload is the data to transmit. The target device number is not repeated in the response because the protocol/manager exchange is already associated with the active protocol selection.
- `IECDeviceFactory` creates from an `IECDeviceConfiguration`; the ID is part of that configuration.
- `IECDeviceManager` has a generic, non-owning device registration/routing interface.
- `IECSystem` no longer carries the deferred printer revision hash.

These header changes define interfaces. They do not imply that all corresponding `.cpp` implementations and concrete devices have already been migrated.

## 3. Agreed target ownership and runtime structure

```text
GUI / external clients
 |
 v
EmulatorController
 |-- EmulatorWorker
 |    |-- C64Machine
 |    |-- IECDeviceFactory
 |    |    |-- IECAsciiPrinter
 |    |    `-- IECFoundryDrive : IECDevice
 |    |         |-- IECFoundryDisk for slot #8
 |    |         |-- IECFoundryDisk for slot #9
 |    |         |-- IECFoundryDisk for slot #10
 |    |         `-- IECFoundryDisk for slot #11
 |    |-- IECSystem
 |    |    |-- IECBus
 |    |    |-- IECProtocol
 |    |    `-- IECDeviceManager
 |    `-- EmulationScheduler
 `-- QThread
```

The drawing shows responsibility and ownership, not literal member declaration order. `IECSystem` owns its internal infrastructure. The factory owns concrete devices. The Foundry Drive owns its internal disks. The manager stores non-owning references only.

Construction order in the worker:

1. `C64Machine`
2. `IECDeviceFactory`
3. `IECSystem`
4. `EmulationScheduler`

Destruction order is the reverse.

## 4. Agreed architecture decisions

### 4.1 Runtime ownership

- `EmulatorController` owns/manages worker and thread only.
- `EmulatorWorker` owns the runtime components.
- `IECSystem` owns `IECBus`, central `IECProtocol` and `IECDeviceManager`.
- `IECDeviceFactory` owns the concrete IEC devices it creates.
- `IECSystem` and the factory remain separate components.
- `IECSystem` does not own a Drive and does not own another `IECSystem`.

### 4.2 Bus, protocol and manager

- `IECBus` handles electrical bus signals only.
- There is one central `IECProtocol`, not one protocol instance per device.
- `IECProtocol` interprets bus activity and emits neutral protocol events. It does not know concrete devices or device types.
- `IECDeviceManager` routes events using IEC addresses and routes device responses back to the protocol.
- The manager does not create, own or destroy devices.
- Devices do not access the bus or protocol directly.

The event/response direction is:

```text
IECBus -> IECProtocol -> IECProtocolEvent
                         |
                         v
                  IECDeviceManager
                         |
                         v
                     IECDevice
                         |
                  IECProtocolResponse
                         |
                         v
                  IECDeviceManager
                         |
                         v
                    IECProtocol -> IECBus
```

### 4.3 Device identity and IEC addresses

- `DeviceId` identifies one configured device object.
- IEC device numbers are protocol addresses, not `DeviceId` values.
- One `IECDevice` may expose multiple IEC addresses.
- Each IEC address may be registered to at most one device.
- Registration of all addresses for one device is atomic.
- Unregistration removes every address belonging to that device.
- The manager stores non-owning pointers and must be cleared before device destruction.

### 4.4 Foundry Drive and disk objects

`IECFoundryDrive` is one `IECDevice`. It may expose any configured subset of IEC device numbers #8, #9, #10 and #11. The GUI may configure only #9, for example; the other slots remain empty.

The drive handles:

- IEC-facing drive behavior and C64 commands, including `LOAD "$"` and the future DOS command set.
- Secondary addresses and channel management.
- Mapping an addressed IEC number to the corresponding internal disk.
- Disk selection and disk swapping.
- Drive-level status and sector-access events.

`IECFoundryDisk` is not an `IECDevice` and has no IEC number. It is an internal drive component that manages one `DiskImage` and provides track/sector read and write access.

The Foundry Drive uses a fixed array of four pointers:

```cpp
static constexpr quint8 FIRST_DEVICE_NUMBER = 8;
static constexpr quint8 LAST_DEVICE_NUMBER = 11;
static constexpr quint8 DEVICE_COUNT = LAST_DEVICE_NUMBER - FIRST_DEVICE_NUMBER + 1;

IECFoundryDisk* m_disks[DEVICE_COUNT] = {};
```

Index 0 maps to #8 and index 3 maps to #11. A populated pointer means that slot exists; `nullptr` means it is not configured. The drive's `supportedDeviceNumbers()` reports only populated slots. The IEC number is derived from the array index and is not stored in `IECFoundryDisk`.

A disk swap changes the disk object associated with a drive slot; the IEC address remains stable. This design also leaves room for future media/storage types such as HDD-like devices without changing the generic manager.

### 4.5 GUI and requests

- The GUI does not own or manipulate live IEC device objects.
- Existing per-drive GUI configuration and image-management concepts should be preserved where practical.
- Device-specific GUI actions travel through generic IEC requests identified by `DeviceId`.
- `IECSystem` routes a request to the target device. A concrete request performs the appropriate type check at the device boundary.
- Generic device events carry the information needed by the GUI, including the IEC number for drive-sector activity.
- Do not add concrete Drive or Disk dependencies to `EmulatorWorker` or `EmulatorController`.

### 4.6 Threading and performance

- Factory operations and device lifecycle changes execute in the emulation thread.
- GUI/external requests cross the thread boundary through controller/worker signals and slots.
- Internal synchronous calls use direct calls.
- No factory work, registration or destruction in the normal emulation-cycle path.
- Avoid per-cycle allocations, temporary containers, unnecessary data copies and verbose logging.
- Use fixed-size/direct lookup structures for small bounded domains where appropriate.
- Each time-sensitive component is advanced exactly once.

## 5. Current implementation findings

The controller, worker and scheduler refactors are committed. `IECSystem` and the agreed interface headers are also committed.

The implementation of `IECDeviceManager`, the central `IECProtocol`/`IECBus` event path and migration of the concrete printer/drive classes are still pending. The existing `.cpp` files must be inspected before each implementation step; do not assume they already match the new headers.

The current source of truth is the repository. Historical findings from before the refactor must not be treated as current implementation facts.

## 6. Remaining implementation sequence

1. **Implement `IECDeviceManager`.**
   - Register one generic device under all numbers returned by `supportedDeviceNumbers()`.
   - Validate all numbers before changing the mapping.
   - Reject invalid numbers, duplicates and conflicts atomically.
   - Unregister every mapping for a device without deleting it.
   - Route protocol events to the device selected by the event's IEC number.
   - Initialize the response to `None` before dispatch.
   - Keep the implementation generic and non-owning.

2. **Finalize protocol/manager interaction.**
   - Verify the event types and response semantics.
   - Ensure protocol selection state and manager routing state cannot become inconsistent.
   - Preserve required LISTEN/TALK and multi-listener semantics.

3. **Adapt `IECBus` and central `IECProtocol`.**
   - Remove per-device protocol instances.
   - Convert bus activity into neutral protocol events.
   - Route responses from the manager back through the central protocol.
   - Preserve the electrical handshake and timing unless separately agreed.

4. **Adapt `IECAsciiPrinter`.**
   - Make it use the generic device interface.
   - Preserve completed-job events and current GUI behavior.
   - Keep printer-specific job semantics inside the printer.

5. **Adapt `IECFoundryDrive` and `IECFoundryDisk`.**
   - Make `IECFoundryDrive` the single registered `IECDevice`.
   - Keep drive command/channel logic in the drive.
   - Keep image and track/sector access in `IECFoundryDisk`.
   - Use the fixed four-entry disk pointer array for #8–#11.
   - Keep disk objects free of IEC device numbers.
   - Preserve current GUI-facing image operations and drive activity events.

6. **Verify integration.**
   - Check reset and device lifecycle.
   - Verify each IEC component is clocked exactly once.
   - Verify printer output and drive access events reach the GUI.
   - Verify one or multiple configured drive slots, including only #9.
   - Verify disk swap does not change the exposed IEC number.
   - Build and test after each coherent step.

## 7. Acceptance criteria

- Controller has no direct IEC bus/device ownership.
- Worker has no concrete IEC device dependencies.
- Scheduler advances the machine and IEC system exactly once per cycle.
- `IECSystem` owns bus, protocol and manager only.
- Factory owns concrete IEC devices.
- Manager performs generic non-owning registration and routing.
- A device can expose multiple IEC addresses.
- The Foundry Drive is registered once for its configured subset of #8–#11.
- `IECFoundryDisk` has no IEC number and is not registered with the manager.
- The drive uses a fixed four-entry disk pointer array; `nullptr` marks an unused slot.
- Disk image and track/sector operations remain inside the disk/storage layer.
- Existing printer and disk GUI workflows are preserved unless a change is explicitly agreed.
- No device is destroyed while the manager still references it.
- No accidental change to IEC handshake timing is introduced.

## 8. Explicit non-goals

- Rewriting IEC electrical timing without evidence that it is necessary.
- Adding physical 1541 CPU/VIA/GCR emulation.
- Registering each `IECFoundryDisk` as a separate IEC device.
- Giving a disk object an IEC number.
- Moving disk ownership into the manager or `IECSystem`.
- Teaching the manager about concrete printer, drive or disk classes.
- Adding a directory browser GUI as part of this architecture refactor.
- Adding new DOS commands as part of the architecture refactor.

## 9. Next action

Implement the `IECDeviceManager` `.cpp` against the committed generic interface. First inspect the current manager and protocol source files, then provide complete replacement files for review. Do not modify the repository directly; the repository owner applies, builds, tests and commits the changes.
