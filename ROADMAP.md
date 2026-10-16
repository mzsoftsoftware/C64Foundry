# Roadmap — C64Foundry

C64Foundry aims to evolve from a Qt-based Commodore 64 emulator into a practical integrated environment for creating, building, running and inspecting C64 software. The milestones below describe intended capabilities, not release dates or promises.

## Current foundation — Implemented, but incomplete

The repository already contains a MOS 6510 execution engine, RAM/ROM and CPU banking, a basic CIA implementation, a partially implemented VIC-II with video output, physical and virtual keyboard handling, a threaded emulator controller and a dockable Qt interface. Configuration persistence and ROM loading are available at the implementation level.

This is an **early emulator foundation**, not a complete C64-compatible runtime or an integrated development environment.

## Milestone 1 — Load and launch C64 programs

**Goal:** Make program loading part of the normal desktop workflow.

- Parse `.prg` files and validate their load addresses.
- Reuse the existing memory-loading functionality through safe worker commands.
- Support BASIC and direct machine-code launch scenarios.
- Add application actions and error reporting.
- Stabilize startup, reset and ROM handling.

**Exit criterion:** A user can select a PRG, load it and launch it without modifying the source code.

## Milestone 2 — Practical emulator compatibility

**Goal:** Execute a useful range of ordinary C64 software.

- Complete core VIC-II display modes, sprites, raster behavior and bus interaction.
- Extend CIA functionality, especially Timer B, TOD and interrupt-related behavior.
- Refine CPU and peripheral timing where real programs require it.
- Expose PAL/NTSC machine configuration.
- Keep sound as a lower-priority feature until it becomes necessary.

**Exit criterion:** Representative text, graphics and interrupt-driven programs run with documented limitations.

## Milestone 3 — Usable machine configuration

**Goal:** Make the implemented configuration model accessible and safe to use.

- Add machine/ROM configuration management in the UI.
- Add a physical keyboard mapping editor, including toggle and multi-key mappings.
- Complete the virtual keyboard's mapping/editor workflow.
- Preserve host-platform-specific mappings without silently converting between platforms.
- Provide clear startup and ROM error messages.

**Exit criterion:** A user can configure ROMs, timing and keyboard behavior without editing configuration storage directly.

## Milestone 4 — Storage and memory expansion

**Goal:** Support file-oriented software and memory-expansion use cases.

- Provide an IEC-like host-backed device for loading, saving, directory access and status/error channels.
- Add disk-image support when required.
- Define a reusable expansion-port interface.
- Implement GeoRAM, followed by REU including DMA.
- Persist expansion and device configuration.

**Exit criterion:** Software can use practical storage services and the selected memory expansions through the emulated machine.

## Milestone 5 — Development-oriented emulator control

**Goal:** Provide a stable interface for tooling rather than requiring direct access to emulator internals.

- Pause/resume and deterministic stepping.
- Safe memory and CPU-register inspection.
- Breakpoints, disassembly and basic debugging views.
- Program-loading and execution-control APIs.
- Initial support for symbols and address mappings.

**Exit criterion:** External development tools can inspect and control an emulated program through defined application interfaces.

## Milestone 6 — Integrated C64 development workflow

**Goal:** Turn C64Foundry into an integrated development environment.

- Projects and source-file management.
- Source editor.
- Assembler/compiler integration.
- Build diagnostics and PRG generation.
- One-step build-and-run in the integrated emulator.
- Symbol-aware and source-level debugging.

**Exit criterion:** A complete **Edit → Build → Run → Inspect → Fix** loop works inside C64Foundry.

## Milestone 7 — Broader compatibility and distribution

**Goal:** Extend the project without sacrificing its development-first focus.

- Improve hardware accuracy based on concrete software needs.
- Add SID audio if required.
- Consider more complete IEC/1541 emulation.
- Expand beyond Linux Wayland where maintainable.
- Improve packaging, configuration UX and documentation.

## Guiding principles

1. **Development-first:** Prioritize capabilities that enable creating and running C64 programs.
2. **Evidence-based status:** Distinguish implemented APIs, partial chip behavior and complete user workflows.
3. **Modular architecture:** Keep emulator devices, host services and development tools separable.
4. **Incremental compatibility:** Implement additional hardware fidelity when software needs demonstrate its value.
5. **No speculative deadlines:** Reorder milestones as implementation discoveries and project requirements evolve.

For current capabilities see [README.md](README.md); for concrete work items see [TODO.md](TODO.md).
