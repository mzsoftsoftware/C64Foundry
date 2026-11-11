# Work Package: IECSystem

**Status:** Implemented and integrated into the IEC architecture refactor  
**Historical purpose:** Introduce the central IEC runtime component  
**Current source of truth:** [IEC architecture](../architecture/IEC.md) and [IEC architecture refactoring](IEC-REFACTOR.md)

## 1. Result

The `IECSystem` work package established the central IEC runtime boundary and has been implemented and committed as part of the broader IEC architecture refactor.

The worker creates `IECSystem`. The system creates and owns its internal infrastructure:

- `IECBus`
- one central `IECProtocol`
- `IECDeviceManager`

The worker separately owns `IECDeviceFactory`. The factory owns concrete IEC devices. `IECSystem` does not own the factory, a concrete Drive or another `IECSystem`.

## 2. Implemented runtime integration

The worker construction order is:

1. `C64Machine`
2. `IECDeviceFactory`
3. `IECSystem`
4. `EmulationScheduler`

Destruction is in reverse order:

1. `EmulationScheduler`
2. `IECSystem`
3. `IECDeviceFactory`
4. `C64Machine`

The scheduler receives `C64Machine*` and `IECSystem*`. Its `clock()` advances the machine and then the IEC system exactly once per cycle. The worker does not independently clock the IEC system.

## 3. Responsibilities

`IECSystem` is responsible for the central IEC infrastructure, its internal connections, reset coordination, device lifecycle coordination and collection of generic IEC events.

`IECDeviceFactory` is a separate worker-owned component responsible for creating and owning concrete devices and their backend resources.

`IECDeviceManager` is a generic non-owning registry/router. It does not create, own or destroy devices.

The worker communicates with IEC through `IECSystem` and does not depend on concrete printer, drive or disk classes.

## 4. Device model clarified after this work package

The original draft referred to drive disk states but did not clearly distinguish the IEC device from its internal disks. The agreed model is now:

- `IECFoundryDrive` is one `IECDevice`.
- The drive may expose any configured subset of IEC numbers #8, #9, #10 and #11.
- The drive owns its internal `IECFoundryDisk` objects.
- `IECFoundryDisk` is not an `IECDevice` and has no IEC number.
- Each `IECFoundryDisk` manages one `DiskImage` and provides track/sector access.
- The drive uses a fixed four-entry pointer array for slots #8–#11; `nullptr` means that slot is unused.
- A disk swap changes the disk object assigned to a slot, not the IEC number exposed by that slot.

These decisions supersede the older wording in this work package about separate disk devices.

## 5. Historical scope and outcome

The original work package proposed:

- centralizing bus, protocol and manager ownership in `IECSystem`;
- moving IEC runtime composition into the worker;
- using one scheduler-controlled IEC clock path;
- keeping concrete device ownership in the factory;
- forwarding generic device events to the worker.

Those architectural goals are now integrated. The remaining generic manager, central protocol and concrete-device migration work is tracked in [IEC-REFACTOR.md](IEC-REFACTOR.md).

## 6. Maintenance note

This document is retained as implementation history. Do not use its original draft interface lists as the current API specification. The current architecture and remaining implementation sequence are maintained in `architecture/IEC.md` and `IEC-REFACTOR.md`.
