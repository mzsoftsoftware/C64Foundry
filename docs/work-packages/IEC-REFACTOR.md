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
 |    |    |-- IECFoundryDrive (Drive A)
 |    |    |    |-- IECFoundryDisk (configured slots)
 |    |    `-- IECFoundryDrive (Drive B)
 |    |         `-- IECFoundryDisk (configured slots)
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

IECFoundryDrive is one IECDevice representing one logical drive system. It has an internal DeviceId, but no IEC device number of its own.

Multiple independent Foundry Drive instances are supported.

Each drive configuration contains:
- The number of available disk slots.
- The start IEC device number.
- An automatically calculated end IEC device number.
The end address is calculated as:

endDeviceNumber = startDeviceNumber + diskCount - 1

The end address is not stored independently.

The current maximum is four disk slots per drive. This is a configurable product limit and must not become an architectural limitation.

The complete IEC address range is reserved by the drive, including empty slots. When the configuration is saved, all reserved ranges must be checked for conflicts with every other configured IEC device, including other drives and printers.

The same IECFoundryDrive pointer is registered with IECDeviceManager under every address in its reserved range. The manager does not know whether a corresponding disk slot is populated.

IECFoundryDisk is not an IECDevice and has no IEC device number. It is an internal component owned by the drive. Each configured disk slot contains one IECFoundryDisk, which manages one DiskImage.

Disk slots are optional. A drive with four available slots may have only one configured disk, for example at IEC #9. Empty slots remain reserved but do not have a disk configuration or disk object.

The GUI displays only configured disks. Empty slots are not displayed as disk entries. The user can add a disk to any free slot within the drive's configured capacity.

The IEC address of a slot is derived from the drive's start address and the slot index. A disk swap changes the disk object associated with a slot, but never changes the slot's IEC address.

The drive handles:
- IEC-facing drive behavior and C64 commands.
- Secondary addresses and channel management.
- Mapping an addressed IEC number to its internal slot.
- Accesses to empty slots.
- Disk selection and disk swapping.
- Drive-level status and sector-access events.
Future drive-specific control commands are intentionally not specified yet. Possible interfaces include I/O addresses or additional IEC secondary-address channels. The drive logic should remain sufficiently independent of this future control interface.

This design does not limit the number of independent drives to one. It also avoids tying the architecture to the current maximum of four disk slots per drive.

### 4.5 GUI and requests

- The GUI does not own or manipulate live IEC device objects.
- Existing per-drive GUI configuration and image-management concepts should be preserved where practical.
- Device-specific GUI actions travel through generic IEC requests identified by `DeviceId`.
- `IECSystem` routes a request to the target device. A concrete request performs the appropriate type check at the device boundary.
- Generic device events carry the information needed by the GUI, including the IEC number for drive-sector activity.
- Do not add concrete Drive or Disk dependencies to `EmulatorWorker` or `EmulatorController`.
The drive configuration is managed as one parent configuration. Individual disk configurations belong to that drive and refer to a slot within its available capacity.
The GUI creates and configures a drive first. It then allows the user to add, edit or remove disk configurations within the drive's available slots. Only configured disks are displayed.
The drive's reserved IEC address range remains occupied even when some or all disk slots are empty. Removing a disk configuration does not release the corresponding IEC address.
The configuration save operation validates all IEC address reservations globally. It rejects invalid address ranges and any overlap with another drive or standalone IEC device.

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
   - Register and unregister individual IEC addresses.
   - Validate address numbers and conflicts.
   - Keep the implementation generic and non-owning.


2. Finalize protocol/manager interaction.
- Verify event types and response semantics.
- Ensure protocol selection state and manager routing state cannot become inconsistent.
- Preserve required LISTEN/TALK and multi-listener semantics.
3. Adapt IECBus and central IECProtocol.
- Remove per-device protocol instances.
- Convert bus activity into neutral protocol events.
- Route responses through the central protocol.
- Preserve electrical handshake and timing unless separately agreed.
4. Adapt IECAsciiPrinter.
- Use the generic device interface.
- Preserve completed-job events and current GUI behavior.
- Keep printer-specific job semantics inside the printer.
5. Redesign Foundry Drive configuration and GUI.
- Introduce a parent IECFoundryDriveConfiguration.
- Store the available disk-slot count and start IEC address.
- Calculate the end address instead of storing it.
- Support multiple independent drives.
- Make disk configurations optional child configurations belonging to a drive.
- Validate all reserved IEC address ranges globally when saving configuration.
- Reserve the complete address range, including empty slots.
- Display only configured disks in the GUI.
- Allow adding, editing and removing disks within the drive's available capacity.
- Keep future drive-control interfaces undecided.
6. Adapt IECDeviceFactory.
- Create and own multiple independent Foundry Drive instances.
- Associate each drive configuration with its drive instance.
- Configure the drive's disk slots from its child disk configurations.
- Create independent printer instances for independent printer configurations.
- Avoid generic device-type deduplication.
7. Adapt IECFoundryDrive and IECFoundryDisk.
- Make IECFoundryDrive the registered IECDevice.
- Register the drive under every address in its reserved range.
- Keep drive command/channel logic in the drive.
- Keep image and track/sector access in IECFoundryDisk.
- Derive each slot's IEC address from the drive start address and slot index.
- Support empty slots and accesses to empty slots.
- Keep disk objects free of IEC device numbers.
- Preserve current GUI-facing image operations and drive activity events.
8. Integrate configuration and lifecycle in IECSystem.
- Coordinate creation, configuration, registration, unregistration and destruction.
- Ensure the manager is cleared before a device is destroyed.
- Keep the factory and IECSystem ownership boundaries intact.
9. Verify integration.
- Check reset and device lifecycle.
- Verify each IEC component is clocked exactly once.
- Verify printer output and drive access events reach the GUI.
- Verify multiple drives with non-overlapping address ranges.
- Verify address conflicts are rejected, including conflicts with empty slots.
- Verify drives with partially populated or completely empty slots.
- Verify disk swap does not change the exposed IEC address.
- Build and test after each coherent step.

## 7. Acceptance criteria

- Multiple independent Foundry Drive instances are supported.
- A Foundry Drive has an internal `DeviceId` but no IEC device number of its own.
- Each drive has a configured disk-slot capacity and start IEC address.
- The end IEC address is calculated from the start address and slot count.
- The current maximum of four slots is not an architectural limitation.
- The complete reserved address range is checked for conflicts against all other IEC devices.
- Empty slots remain part of the reserved address range.
- The same drive pointer is registered under every address in its reserved range.
- Only configured disk slots are displayed in the GUI.
- Disk configurations are optional and belong to a drive.
- Slot IEC addresses are derived from the drive start address and slot index.
- `IECFoundryDisk` has no IEC number and is not registered with the manager.
- Disk image and track/sector operations remain inside the disk/storage layer.
- Disk swapping does not change the slot's IEC address.
- The future drive-control interface (I/O addresses or IEC secondary-address channels) remains undecided.

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


Redesign the Foundry Drive configuration model and GUI before continuing the concrete Foundry Drive runtime integration.

First inspect the current configuration classes, configuration persistence and GUI device-configuration workflow. Define the parent drive configuration and optional child disk configurations, then implement drive creation and configuration in the GUI.

Do not modify the repository directly. Provide complete replacement files or focused patches for review. The repository owner applies, builds, tests and commits the changes.
