# Roadmap — C64Foundry

C64Foundry is evolving from a Qt-based Commodore 64 emulator into a practical environment for creating, building, running and inspecting C64 software.

The roadmap reflects the current implementation and the agreed development priorities. It is a plan, not a release schedule. Features are considered complete only when the required behavior is implemented and usable, not merely because supporting classes, registers or UI elements exist.

## Current foundation — Implemented, but incomplete

The repository currently contains:

- MOS 6510 execution engine, RAM/ROM handling and CPU memory banking.
- Initial CIA and VIC-II implementations with video output.
- Physical and virtual keyboard handling.
- Threaded emulator controller and dockable Qt interface.
- Persistent configuration and ROM loading.
- IEC bus and protocol implementation.
- IEC device manager with support for multiple devices.
- ASCII printer device with GUI output.
- Foundry Disk device with D64 image insertion, ejection and saving.
- D64 sector reading and directory program generation.
- IEC transfer of the generated directory, including `LOAD "$",8`.
- Reading PRG files from D64 images through directory and sector chains.

This is an early emulator and storage foundation. It is not yet a complete C64 emulator, a complete 1541/1541-II drive emulator or an integrated development environment.

## Milestone 1 — Complete the disk GUI workflow

**Goal:** Make the existing D64 functionality visible and useful from the desktop application.

- Show the current Track and Sector information for the selected disk device.
- Add a `DIR` action to the disk device GUI.
- Generate the directory when the user requests it, using the currently attached image and its current in-memory state.
- Display the directory in a readable form, including filenames, file types, block counts and free blocks.
- Ensure the displayed directory is refreshed after image insertion/ejection and after later disk modifications.
- Keep directory generation in the drive/storage layer; the GUI should present the result rather than duplicate D64 parsing logic.
- Provide useful error reporting for missing images, invalid sectors and malformed directory chains.

**Exit criterion:** A user can attach a D64 image, inspect its current Track/Sector state and request a directory display that reflects the currently loaded image.

## Milestone 2 — 1541/1541-II compatibility

**Goal:** Turn the current D64-backed IEC device into a useful implementation of the standard Commodore 1541/1541-II disk-drive interface and DOS behavior.

Use VICE as the primary reference for observable drive behavior, protocol details and compatibility tests. VICE should guide behavior and edge cases; its complete architecture does not need to be copied.

### IEC protocol and channels

- Complete LISTEN/TALK handling and the relevant IEC state transitions.
- Implement secondary-address handling for the supported DOS channels.
- Implement the command channel (secondary address 15).
- Implement the data channel and the status/error channel.
- Handle channel open/close and command sequencing consistently.
- Return appropriate DOS status codes and error messages.
- Handle invalid commands, missing files, missing images and invalid channel operations predictably.

### Disk DOS and directory behavior

- Implement the standard disk commands needed for normal 1541/1541-II use.
- Implement directory and BAM handling robustly.
- Support file lookup, wildcard/prefix behavior and file-type handling according to the expected DOS semantics.
- Implement disk initialization and validation behavior where required.
- Keep the D64 image format and sector-level storage logic separate from IEC protocol handling.

### Compatibility and validation

- Compare externally visible behavior with VICE.
- Use real C64 BASIC commands and small test programs to validate protocol and DOS behavior.
- Verify both successful operations and DOS error responses.
- Keep ordinary DOS compatibility separate from fastloader and direct-drive-hardware compatibility.

**Exit criterion:** Common C64 BASIC disk operations behave consistently with a standard 1541/1541-II when using supported D64 images, with meaningful DOS status and error responses.

## Milestone 3 — Disk writing and file operations

**Goal:** Support normal read/write workflows on D64 images.

- Implement sector writing with track/sector validation.
- Update the in-memory image state and modified-state tracking correctly.
- Persist modified images safely to their original or selected image file.
- Implement `SAVE` for PRG files.
- Implement file creation, replacement and deletion.
- Update directory entries and BAM allocation information consistently.
- Implement disk formatting and initialization.
- Handle disk-full conditions and invalid or exhausted sector chains.
- Ensure that image saving does not silently discard changes after errors.

**Exit criterion:** Standard PRG files can be loaded, saved, replaced and deleted, and disks can be formatted and saved without corrupting their directory or BAM structures.

## Milestone 4 — Additional DOS file types

**Goal:** Extend file handling beyond PRG files.

- Implement SEQ file reading and writing.
- Implement REL file support, including record-length and side-sector handling.
- Implement the required DOS channel semantics for these file types.
- Validate compatibility with VICE and representative C64 software.
- Keep unsupported file types and operations explicit rather than silently treating them as PRG files.

**Exit criterion:** The supported SEQ and REL operations behave consistently with the corresponding 1541 DOS behavior.

## Milestone 5 — Decide the required depth of drive emulation

**Goal:** Define how far C64Foundry must emulate the physical 1541/1541-II drive hardware.

The current D64 sector-based approach is suitable for standard sector-oriented DOS functionality. It does not represent all physical disk characteristics.

Evaluate the need for:

- G64 and/or P64 image support.
- GCR encoding and decoding.
- Track and half-track representation.
- Drive CPU, RAM and ROM emulation.
- Two 6522 VIA instances and their drive-side behavior.
- Rotation, synchronization and other timing-dependent behavior.
- Copy-protection and non-standard disk layouts.
- Fastloaders and software that accesses drive hardware directly.

Do not implement these features merely to reproduce VICE's internal architecture. Add them when the intended compatibility level or concrete software requires them.

**Exit criterion:** The project has an explicit, tested compatibility target for standard DOS operation versus hardware-level and fastloader compatibility.

## Milestone 6 — Complete C64 hardware compatibility

**Goal:** Improve the emulated C64 hardware based on real software requirements.

### VIC-II

- Complete bitmap, multicolor and extended-background display modes.
- Implement sprite data fetch and rendering.
- Implement sprite expansion, multicolor, priority and collision behavior.
- Refine raster timing, scrolling, border behavior and bus arbitration.

### CIA

- Complete Timer A and Timer B behavior.
- Implement TOD clock and alarm behavior.
- Implement serial shift register and relevant input/output signals.
- Complete interrupt and port integration.

### Other hardware

- Add SID audio synthesis and host audio output when the core functionality is sufficiently stable.
- Refine CPU and peripheral timing based on compatibility tests.
- Expose PAL/NTSC selection through the application configuration.

**Exit criterion:** A documented set of representative C64 programs runs with the expected graphics, input, interrupt and timing behavior.

## Milestone 7 — Memory expansion and additional devices

**Goal:** Support selected hardware expansions through a reusable architecture.

- Define and implement a reusable expansion-device interface.
- Implement GeoRAM.
- Implement REU registers, transfers and DMA interaction.
- Add persistent configuration and UI management for supported expansions.
- Add further IEC devices only when they serve a concrete compatibility or development requirement.

**Exit criterion:** Supported expansion devices can be configured, persisted and used by software through the emulated machine.

## Milestone 8 — Development-oriented emulator control

**Goal:** Provide stable interfaces for development tools.

- Synchronized machine-state access.
- Safe memory and CPU-register inspection.
- Controlled pause/resume and deterministic stepping.
- Breakpoints and execution until a specified condition.
- Disassembly and memory/register views.
- Program-loading and execution-control APIs.
- Symbol and address-mapping integration points.

**Exit criterion:** Development tools can inspect and control an emulated program through defined application interfaces.

## Milestone 9 — Integrated C64 development workflow

**Goal:** Turn C64Foundry into an integrated development environment.

- Project and workspace management.
- Source editor and file navigation.
- Assembler/compiler integration.
- Build diagnostics and PRG generation.
- One-step build-and-run in the integrated emulator.
- Symbol-aware and source-level debugging.

**Exit criterion:** A complete **Edit → Build → Run → Inspect → Fix** workflow works inside C64Foundry.

## Milestone 10 — Platform support and distribution

**Goal:** Improve usability and distribution without compromising the emulator architecture.

- Expand beyond Linux Wayland where maintainable.
- Improve packaging and release automation.
- Improve configuration and device-management workflows.
- Improve documentation and localization.
- Add regression tests for stable hardware, IEC and disk functionality.

## Guiding principles

1. **Compatibility before feature count:** Implement behavior that real C64 software needs.
2. **VICE as a reference:** Use VICE to compare observable behavior and investigate edge cases, without blindly copying its architecture.
3. **Clear compatibility levels:** Standard D64/DOS compatibility is not the same as GCR, copy-protection, fastloader or cycle-exact drive compatibility.
4. **Separation of concerns:** Keep IEC protocol, DOS behavior, disk-image storage and GUI presentation in separate layers.
5. **Fresh state:** UI views such as the disk directory should be generated from the current image state, not stale cached output.
6. **Evidence-based status:** Distinguish implemented APIs, partial hardware behavior and complete user workflows.
7. **Incremental development:** Use buildable, testable steps and commit stable checkpoints.
8. **No speculative deadlines:** Reorder milestones as implementation discoveries and project requirements evolve.

For current capabilities see [README.md](README.md); for concrete implementation tasks see [TODO.md](TODO.md).

