# Emulator Runtime and Threading

## Purpose

This document records the responsibilities and ownership of `EmulatorController`, `EmulatorWorker`, `IECSystem`, `IECDeviceFactory` and `EmulationScheduler`.

It distinguishes the currently committed runtime structure from responsibilities that remain inside the ongoing IEC-device migration.

## `EmulatorController`

The controller is the external control boundary for the emulator runtime. It is not limited to GUI-to-emulator communication; other clients may connect to its request and notification interface.

Responsibilities:

- Own and manage the worker and its `QThread`.
- Forward emulator and IEC requests to the worker using the established queued signal/slot mechanism.
- Receive worker notifications and expose them through controller signals.
- Keep request/response routing separate for emulator requests and IEC requests.
- Avoid owning or directly managing IEC infrastructure.
- Avoid constructing concrete IEC devices.
- Avoid exposing the machine or its internal runtime objects.

The controller communicates through signals and slots. It does not need to know about `IECBus`, `IECProtocol`, `IECDeviceManager`, `IECDeviceFactory` or individual IEC device classes.

The controller does not maintain a second copy of worker state or own program-load sequencing. Higher-level callers compose the required emulator requests.

## `EmulatorWorker`

The worker is the composition root for runtime objects that execute in the emulation thread.

It owns and initializes:

- `C64Machine`
- `IECDeviceFactory`
- `IECSystem`
- `EmulationScheduler`

The construction order is:

1. `C64Machine`
2. `IECDeviceFactory`
3. `IECSystem`
4. `EmulationScheduler`

The destruction order is the reverse:

1. `EmulationScheduler`
2. `IECSystem`
3. `IECDeviceFactory`
4. `C64Machine`

This order matters: `IECSystem` must be torn down while the factory-owned devices still exist, so manager registrations and non-owning references can be removed safely before the factory destroys those devices.

The worker:

- Runs the emulation loop and controls timing.
- Processes queued emulator and IEC requests.
- Delegates cycle advancement to `EmulationScheduler`.
- Uses `IECSystem` for IEC reset, requests and event collection.
- Forwards generic IEC system/device event batches to the controller.
- Does not know concrete IEC devices, the drive implementation or disk-image classes.

The worker's destructor does not perform an independent emulation shutdown sequence. The run-loop shutdown path is responsible for stopping emulation and performing the final machine power-off.

## `IECSystem`

`IECSystem` is the central IEC runtime facade.

It owns:

- `IECBus`
- one central `IECProtocol`
- `IECDeviceManager`

It is constructed by the worker with the machine and factory dependencies it needs. It creates and connects its own internal IEC infrastructure.

`IECSystem` does not own `IECDeviceFactory` and does not own a concrete drive. It coordinates device lifecycle and routing with the factory and manager.

Responsibilities include:

- Connecting the IEC bus to the C64 machine.
- Advancing the central IEC protocol through one runtime path.
- Coordinating protocol events, manager dispatch and protocol responses.
- Resetting IEC protocol/device state when requested.
- Managing configured device registration and removal in cooperation with the factory.
- Collecting generic system and device events for the worker.

The system must not create an `IECSystem` inside itself or maintain a second owner for its internal infrastructure.

## `IECDeviceFactory`

The factory is owned by the worker. It creates and owns concrete IEC devices and any shared backend resources required by them.

- Device creation and destruction occur in the emulation thread.
- The factory does not perform IEC protocol routing.
- The factory does not participate in the per-cycle hot path.
- Devices are unregistered from the manager before the factory destroys them.

The Foundry Drive is one factory-owned IEC device. Its internal disk objects are owned by the drive, not separately registered with the manager.

## `EmulationScheduler`

The scheduler is constructed with the machine and IEC system:

```cpp
explicit EmulationScheduler(C64Machine* ptrMachine, IECSystem* ptrIECSystem);
```

Its public execution operations are `clock()` and `runCycles(quint64 cycles)`.

For each emulated cycle, `clock()` advances:

1. `C64Machine::clock()`
2. `IECSystem::clock()`

`runCycles()` repeatedly invokes `clock()`.

The worker does not call `IECSystem::clock()` separately. This avoids duplicate IEC advancement and keeps the machine/IEC ordering in one place.

## Request and event flow

- External callers submit `EmulatorRequest` or `IECRequest` objects through `EmulatorController`.
- The controller forwards requests to the worker through signals/slots.
- The worker processes requests in the emulation thread and delegates IEC-specific operations to `IECSystem`.
- `IECSystem` returns generic events to the worker.
- The worker forwards event batches through controller-facing signals.
- The GUI and other clients do not receive live pointers to worker-owned runtime objects.

Device-specific GUI operations use the generic IEC request mechanism and `DeviceId`. Concrete request execution performs any required type validation at the device boundary; the worker and controller do not need concrete device dependencies.

## Reset and shutdown

`EmulatorWorker` delegates IEC reset semantics to `IECSystem::reset()` for power-on and hard-reset operations. It does not reset individual concrete devices itself.

The worker's run loop owns final machine shutdown. Object destruction then follows the dependency order described above.

## Thread affinity and lifecycle

- Runtime objects are created and destroyed in the worker's controlled lifecycle.
- GUI/external requests cross the thread boundary through the controller/worker request path.
- No GUI widget or GUI-owned object is accessed directly from the worker.
- Factory operations and device lifecycle changes execute in the emulation thread.
- The manager must never retain pointers to destroyed devices.
- The system is destroyed before the factory so registrations can be removed while devices remain alive.

## Performance rules

The hot path is the repeated execution path used while the emulator is running.

- No device creation or destruction.
- No registration/unregistration.
- No dynamic list reconstruction.
- No unnecessary signal/slot dispatch for synchronous per-cycle work.
- No avoidable temporary allocations or data copies.
- No verbose logging.
- Use direct calls and compact, predictable data structures for bounded domains.
- Optimize based on profiling and measurements; do not sacrifice correctness for speculative micro-optimizations.

## Implementation status

Committed and completed in the current refactor:

- `EmulatorController` ownership and request/notification boundary.
- `EmulatorWorker` runtime ownership and lifecycle.
- `EmulationScheduler` central machine/IEC clock path.
- `IECSystem` as the central IEC infrastructure owner.

The generic device-manager implementation and migration of the old protocol/device implementations are still in progress. See [IEC-REFACTOR](../work-packages/IEC-REFACTOR.md) for the current sequence.
