# Emulator Runtime and Threading

## Purpose

This document records the intended responsibilities of `EmulatorController`, `EmulatorWorker`, `IECSystem`, `IECDeviceFactory` and `EmulationScheduler`.

## `EmulatorController`

The controller is the GUI-facing boundary for the emulator runtime.

Responsibilities:

- Create, start, stop and destroy the worker thread and worker.
- Forward GUI requests to the worker using the established thread-safe mechanism.
- Receive worker notifications and expose them to the GUI.
- Avoid owning or directly managing IEC infrastructure.
- Avoid constructing concrete IEC devices.

The controller should know about the worker and thread, not about `IECBus`, `IECProtocol`, `IECDeviceManager` or individual IEC device classes.

## `EmulatorWorker`

The worker is the composition root for runtime objects that execute in the emulation thread.

Target responsibilities:

- Own and initialize the `C64Machine`.
- Own and initialize the `EmulationScheduler`.
- Own and initialize `IECSystem`.
- Own and initialize `IECDeviceFactory`.
- Connect the machine to the IEC bus exposed by `IECSystem`.
- Connect the factory to the device-registration interface exposed by `IECSystem`/`IECDeviceManager`.
- Process configuration and device-management requests in the emulation thread.
- Publish required state and output to the controller/GUI through the existing worker communication mechanism.

The worker should coordinate components without absorbing their internal responsibilities.

## `IECSystem`

`IECSystem` owns the IEC infrastructure and connects its internal components. See [IEC architecture](IEC.md).

The worker owns the `IECSystem` object. The system owns its bus, protocol and manager.

## `IECDeviceFactory`

The factory is owned by the worker and creates/owns concrete IEC peripherals. It is connected to the manager's registration interface.

All factory operations that create, configure, register, unregister or destroy IEC devices must execute in the emulation thread.

The GUI sends configuration data or a device-management request; it never passes a live IEC device object across the thread boundary.

## `EmulationScheduler`

The scheduler controls the execution cadence. The final relationship between scheduler, machine and IEC runtime must be implemented so each time-sensitive component is advanced exactly once and in the correct order.

The current scheduler behavior must be inspected before changing it. In particular, if it currently clocks IEC devices individually, the refactor must remove duplicate ownership/clocking rather than simply adding another clock path through the manager.

Do not introduce Qt signals/slots for synchronous per-cycle calls. Use direct calls within the emulation thread.

## Thread-affinity and lifecycle

- Runtime objects are created and destroyed in a controlled lifecycle associated with the emulation thread.
- GUI requests are transferred to the worker through the existing queued/thread-safe command path.
- No GUI widget or GUI-owned object is accessed directly from the worker.
- The worker must stop emulation before tearing down runtime components.
- The factory must unregister devices before destroying them.
- The manager must not retain pointers to destroyed devices.
- Destruction order must respect dependencies: devices are unregistered/destroyed while the manager and IEC infrastructure are still alive; then the IEC infrastructure is torn down.

## Performance rules

The hot path is the repeated execution path used while the emulation is running.

Keep it lean:

- No device creation or destruction.
- No registration/unregistration.
- No dynamic list reconstruction.
- No unnecessary signal/slot dispatch.
- No unnecessary temporary allocations or data copies.
- No verbose logging.
- Use direct calls for synchronous internal work.
- Use compact, predictable data structures for bounded domains such as IEC device addresses.

Optimize based on profiling and measurements. Do not sacrifice correctness or maintainability for speculative micro-optimizations.
