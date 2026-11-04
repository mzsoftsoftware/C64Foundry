# C64Foundry Architecture

## Purpose

C64Foundry is a Qt 6/C++ Commodore 64 emulator. The application combines the emulated C64 hardware, an execution environment, host input/output, and configurable peripheral devices.

This document describes the intended separation of responsibilities. It is not a claim that every part of the target architecture has already been implemented.

## Main areas

| Area | Responsibility |
|---|---|
| `C64` | Emulated C64 hardware and machine-level behavior |
| `Emulator` | Worker thread, execution scheduling and coordination of the running machine |
| `Input` | Host and virtual keyboard input |
| `Output` | Host-side video output and related presentation |
| `Gui` | Qt Widgets user interface and user interaction |
| `Configuration` | Persistent application and machine configuration |
| `App` | Application startup and top-level GUI composition |
| `Resources` | Embedded resources such as ROMs and licenses |

The exact source-directory structure is defined by the repository, not by this overview.

## Layering principles

1. The GUI expresses user intent and presents state. It does not directly create or manipulate objects that belong to the emulation thread.
2. The `EmulatorController` is the GUI-facing control boundary for the emulator worker and its thread.
3. The `EmulatorWorker` owns and coordinates the runtime components that belong to the emulation thread.
4. Hardware and peripheral subsystems expose narrow interfaces and should not depend on GUI classes.
5. Ownership must be explicit. A component that merely references another component must not destroy it.
6. Registration, configuration and object creation belong to setup or controlled reconfiguration paths, not to the per-cycle hot path.
7. The emulation hot path should use direct calls and compact data structures. Avoid unnecessary allocations, copies, container reconstruction, logging and cross-thread signaling in cycle-sensitive code.

## Emulator runtime target

The intended runtime ownership is:

- `EmulatorController`: owns/manages the worker and its `QThread`; forwards GUI requests to the worker.
- `EmulatorWorker`: owns the emulation runtime components, including `IECSystem` and `IECDeviceFactory`.
- `EmulationScheduler`: controls the execution cadence and advances the machine/runtime according to the established scheduling model.
- `C64Machine`: represents the emulated C64 machine and its hardware.

The worker is the composition root for runtime components. The controller should not need to know about `IECBus`, `IECProtocol` or individual IEC devices.

## Ownership versus coordination

Keep these concepts separate:

- **Ownership:** determines which component is responsible for destroying an object.
- **Registration:** makes an object available to another component through a defined interface.
- **Coordination:** determines when work is performed and in what order.
- **Lookup/dispatch:** routes an event or request to the relevant object.

For the IEC subsystem, the factory owns concrete devices, while the device manager only keeps non-owning references and address mappings. The scheduler must not become a second owner of devices.

## Performance

Correctness and clear ownership come first, but runtime design must avoid avoidable overhead:

- Do not allocate memory in the normal emulation-cycle path unless required by the operation.
- Do not rebuild device lists or address maps during normal operation.
- Prefer fixed-size/direct lookup structures when the domain is small and bounded.
- Avoid Qt signals/slots for synchronous internal operations that happen on every emulation cycle.
- Keep logging out of timing-sensitive paths or make it explicitly controllable.
- Do not add abstraction layers that merely forward calls without providing a real responsibility.
- Measure before applying low-level optimizations; retain readable code where performance is not affected.

## Status of this document

This is the agreed architectural direction. When implementation details differ, update this document only after the design has been discussed and accepted.
