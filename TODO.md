# TODO — C64Foundry

This is an implementation-focused task list based on the current source tree. Items marked **Partial** already have supporting code but are not complete; **Missing** means no end-to-end implementation was found. Completed capabilities are documented in [README.md](README.md) rather than repeated as tasks here.

## P0 — Make the existing emulator useful for loading software

- [ ] **Missing:** Implement `.prg` parsing: validate the two-byte little-endian load address, reject malformed/oversized files and extract payload bytes.
- [ ] **Partial:** Connect the existing `C64Machine::loadProgram(address, data)` memory-copy API to a thread-safe emulator-controller request.
- [ ] **Missing:** Add a Load PRG action, file picker and clear success/failure reporting.
- [ ] **Missing:** Define separate BASIC and machine-code launch flows, including how the BASIC pointers are updated and when a direct execution address is allowed.
- [ ] **Partial:** Review machine power-on/reset semantics, ROM-load failure handling and predictable initialization of attached devices.
- [ ] **Partial:** Review CPU/IRQ/NMI and VIC-II BA/AEC interaction for compatibility with actual programs.

## P1 — Finish the hardware needed for practical compatibility

### VIC-II

- [ ] **Partial:** Complete character display timing, character fetch/shift-register behavior, scrolling and border transitions.
- [ ] **Missing:** Implement multicolor text, extended-background text, high-resolution bitmap and multicolor bitmap display modes.
- [ ] **Partial:** Complete raster register and interrupt behavior, including accurate acknowledge and timing details.
- [ ] **Partial:** Implement sprite data fetch, positioning, enable/disable, expansion, multicolor and priority; existing sprite register storage is not sprite rendering.
- [ ] **Missing:** Implement sprite/sprite and sprite/background collisions and corresponding interrupts.
- [ ] **Partial:** Refine memory-fetch contention, badlines and PAL/NTSC raster timing.

### CIA and machine I/O

- [ ] **Partial:** Finish Timer A operating modes and underflow/interrupt corner cases.
- [ ] **Missing:** Implement Timer B, its count modes and Timer A chaining.
- [ ] **Missing:** Implement time-of-day clock, alarm and related registers.
- [ ] **Missing:** Implement serial shift register, CNT/FLAG input and PB6/PB7 timer outputs.
- [ ] **Partial:** Finish CIA interrupt behavior and port-level integration, including relevant CIA2 functions.
- [ ] **Missing:** Define intentional SID-register behavior (minimal placeholder first; sound later).

## P1 — Improve configuration and host input

- [ ] **Partial:** Add UI to list, create, select, edit and delete persistent machine configurations.
- [ ] **Partial:** Add ROM-set selection and validation in the UI; VICE ROM detection currently uses a fixed Linux path.
- [ ] **Partial:** Expose PAL/NTSC selection and persist the chosen timing mode.
- [ ] **Partial:** Add a keyboard mapping editor using the existing override storage, including multiple C64 keys and momentary/toggle modes.
- [ ] **Partial:** Define safe input-state reset when switching mappings/configurations and when application focus changes.
- [ ] **Partial:** Complete the virtual keyboard's non-input/editor mode and provide the planned mapping-preview workflow.
- [ ] **Missing:** Add separate keyboard backends/mappings for X11, Windows and macOS only when those platforms become supported.
- [ ] **Partial:** Improve actionable startup feedback for invalid configurations and ROM-loading failures.

## P2 — Files, storage and expansion devices

- [ ] **Missing:** Define a reusable C64 program/file abstraction and an application-facing save/export workflow.
- [ ] **Missing:** Design a practical IEC/device layer supporting LOAD, SAVE, directory listing and error-channel semantics.
- [ ] **Missing:** Implement a simple host-backed storage device before considering cycle-accurate 1541 emulation.
- [ ] **Missing:** Add D64 image support when needed by the chosen storage approach.
- [ ] **Missing:** Introduce an expansion-device interface for IO1/IO2, cartridge signals and address-space integration.
- [ ] **Missing:** Implement GeoRAM register/bank selection and expansion RAM.
- [ ] **Missing:** Implement REU registers, transfers, DMA interaction and configurable memory capacity.
- [ ] **Missing:** Add UI and persistent settings for storage and expansion devices.

## P2 — Development-facing emulator API

- [ ] **Partial:** Introduce explicit pause/resume and synchronized machine-state access on top of the existing start/stop worker controls.
- [ ] **Partial:** Expose safe read/write memory operations, CPU registers and program loading through the emulator controller.
- [ ] **Missing:** Add instruction stepping, breakpoints and controlled execution until a specified condition.
- [ ] **Missing:** Provide a minimal memory viewer, register viewer and disassembly interface.
- [ ] **Missing:** Define source-address and symbol integration points for future tools.

## P3 — Integrated development environment

- [ ] **Missing:** Implement project/workspace creation and persistence.
- [ ] **Missing:** Add a source editor and file navigation.
- [ ] **Missing:** Integrate assembler/compiler execution and collect diagnostics.
- [ ] **Missing:** Build PRG artifacts and launch them directly in the emulator.
- [ ] **Missing:** Add symbol-aware navigation, source-level breakpoints and source-to-address mapping.
- [ ] **Missing:** Implement the complete Edit → Build → Run → Inspect workflow.

## Later / demand-driven

- [ ] Add SID audio synthesis and host audio output.
- [ ] Consider more accurate IEC timing and complete 1541 drive emulation only when required.
- [ ] Extend host-platform support after the Linux Wayland workflow is stable.
- [ ] Improve packaging, distribution, localization and end-user configuration workflows.

## Priority principle

Prefer work that makes real C64 programs and the future integrated development workflow possible. Do not treat a register's existence or a UI placeholder as a completed hardware feature.
