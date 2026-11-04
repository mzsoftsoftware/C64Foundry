# Work Package: IEC Architecture Refactoring

**Status:** Design agreed; implementation not started  
**Last updated:** 2026-11-04  
**Current step:** Refactor `EmulatorController` and `EmulatorWorker`

## 1. Goal

Restructure the emulator runtime and IEC subsystem so that:

- The controller is independent of IEC implementation details.
- The worker owns the runtime components used by the emulation thread.
- IEC bus, protocol and device management are separated.
- Concrete IEC devices are created and owned by a factory.
- The device manager only maps IEC addresses to generic devices and dispatches communication.
- The emulation hot path remains efficient.

The refactor must preserve existing printer and disk behavior unless a behavior change is explicitly included in a later work package.

## 2. Agreed target structure

```text
GUI
 |
 v
EmulatorController
 |-- EmulatorWorker
 |    |-- C64Machine
 |    |-- EmulationScheduler
 |    |-- IECSystem
 |    |    |-- IECBus
 |    |    |-- IECProtocol
 |    |    `-- IECDeviceManager
 |    `-- IECDeviceFactory
 |         |-- IECAsciiPrinter
 |         `-- IECFoundryDrive
 |              |-- internal disk/address state #8
 |              |-- internal disk/address state #9
 |              `-- internal disk/address state #10 ...
 `-- QThread
```

This is a responsibility/ownership diagram, not a final declaration of C++ member order or construction sequence.

## 3. Decisions

### Runtime ownership

- `EmulatorController` manages the worker and its thread.
- `EmulatorController` does not own or expose `IECBus`.
- `EmulatorWorker` owns `IECSystem` and `IECDeviceFactory`.
- `IECSystem` owns `IECBus`, `IECProtocol` and `IECDeviceManager`.
- `IECDeviceFactory` owns the concrete IEC devices it creates.

### IEC separation

- `IECBus` handles electrical bus signals only.
- One central `IECProtocol` handles IEC signaling and timing.
- `IECProtocol` does not know devices or device types.
- `IECDeviceManager` maps IEC addresses to generic `IECDevice` instances, dispatches protocol events and routes device transmit requests back to the protocol.
- Devices do not access the bus or protocol directly; they communicate through the manager.
- The manager does not create, own or destroy devices.

### Device registration

- A device may expose multiple IEC addresses.
- Each IEC address can be registered to at most one device.
- `IECFoundryDrive` manages its own IEC addresses (for example #8, #9, #10 and #11).
- The drive remains responsible for its internal disk objects and image state.
- Registration supplies a generic device reference and its IEC-address list. The manager does not know that the object is a drive.
- The factory registers a device after construction/configuration and unregisters it before destruction.
- Exact registration signatures and error handling remain part of the interface-design step.

### Threading

- Factory operations and device lifecycle changes execute in the emulation thread.
- The GUI sends configuration/request data to the controller; the controller forwards it to the worker.
- Live device objects are not created or manipulated by the GUI.
- Internal synchronous runtime calls should be direct calls, not Qt signal/slot traffic on every cycle.

### Performance

- No factory work, registration or destruction in the normal emulation-cycle path.
- Prefer direct lookup for the small IEC address domain (0–15), subject to interface validation.
- Avoid per-cycle allocations, temporary containers, unnecessary data copies and verbose logging.
- Ensure protocol/device clocking occurs exactly once; do not retain duplicate scheduler and manager clock paths.

## 4. Current implementation findings

The following findings were recorded from the repository inspection before this work package was written. Recheck the source files before implementation because the repository may have changed.

- `EmulatorController` currently creates/owns `IECBus` and passes it to `EmulatorWorker`.
- `EmulatorWorker` currently creates `C64Machine`, `EmulationScheduler`, `IECFoundryDrive` and `IECDeviceManager`.
- The worker currently has direct knowledge of concrete printer/drive-related types for publishing GUI state/output.
- The existing `IECDeviceManager` has concrete device creation/ownership responsibilities that conflict with the agreed target.
- The existing `IECFoundryDisk` and `IECAsciiPrinter` each currently own an `IECProtocol` instance.
- The scheduler currently has its own IEC-device clocking behavior; this must be inspected carefully to prevent duplicate ticking.
- `IECFoundryDrive` manages multiple drive states keyed by IEC device number and owns/manages the associated disk/image state.

These are baseline observations, not a substitute for re-reading the current code.

## 5. Scope of the first implementation step

### Included

- Inspect the current `EmulatorController.h/.cpp` and `EmulatorWorker.h/.cpp`.
- Move IEC bus ownership out of `EmulatorController`.
- Change worker construction so the controller no longer passes an `IECBus*`.
- Establish the worker's ownership/lifecycle structure for `IECSystem` and `IECDeviceFactory`.
- Preserve existing controller/worker GUI communication, command handling, scheduler behavior and current IEC behavior as far as possible.
- Keep the change buildable and limited to the controller/worker boundary.

### Not included yet

- Implementing the new `IECDeviceManager`.
- Rewriting `IECBus` or `IECProtocol`.
- Removing per-device protocol instances.
- Adapting `IECAsciiPrinter` or `IECFoundryDrive`.
- Changing IEC electrical timing or handshake behavior.
- Reworking disk ownership or disk-image handling.
- Changing IEC event semantics or DOS/channel behavior.

The first step may need temporary compatibility wiring so the project remains buildable before the new `IECSystem` and factory are fully implemented. Any such transitional state must be explicit and removed in the later integration step.

## 6. Implementation sequence

1. Re-fetch and inspect the current four controller/worker files and relevant constructor/destructor call sites.
2. Inspect `EmulatorThread`, `EmulationScheduler`, `C64Machine` bus attachment and all worker/controller IEC accessors/signals/slots.
3. Agree exact member ownership, construction order and destruction order.
4. Refactor `EmulatorController` so it manages only worker/thread and GUI-facing requests/notifications.
5. Refactor `EmulatorWorker` as the runtime composition root.
6. Build and resolve only issues caused by this step.
7. Commit the stable controller/worker refactor.
8. Continue with `IECSystem` and `IECDeviceFactory` integration as a separately reviewable step.

## 7. Acceptance criteria

- `EmulatorController` has no `IECBus` member and does not create or destroy the bus.
- The worker constructor no longer receives an `IECBus*` from the controller.
- Worker and thread start/stop behavior remains correct.
- Existing GUI requests and worker notifications remain functional.
- The application builds with the project's strict compiler warnings.
- No IEC device is clocked twice.
- No IEC device is destroyed while the manager still retains its address mapping.
- No change to IEC handshake behavior is introduced unintentionally.

## 8. Open points for interface-design work

These must be settled before implementing the relevant components:

- Exact `IECSystem` public interface and whether it exposes references/pointers to the bus and manager.
- Exact `IECDeviceFactory` API for adding/removing configured devices.
- How device address lists are exposed generically by `IECDevice`.
- Whether the manager itself clocks registered devices or the scheduler clocks the manager/system once.
- How worker-to-GUI printer output and drive activity/status are exposed without concrete device dependencies in the worker.
- Exact protocol event structure, including command type, IEC address, secondary address, data byte and EOI semantics.
- Failure behavior for duplicate/invalid device addresses and failed device construction.

## 9. Next action

Start a new implementation chat with this document and the current repository state. The first task is to inspect the current `EmulatorController` and `EmulatorWorker` implementation and produce the exact first-step changes. Do not implement the manager/protocol/device refactor in that first step.
