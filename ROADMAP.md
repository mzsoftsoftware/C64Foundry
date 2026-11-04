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
- Queued disk-sector access events and disk GUI read/write activity indicators.
- Track/Sector information in the disk device GUI.

This is an early emulator and storage foundation. It is not yet a complete C64 emulator or a complete implementation of the standard 1541 DOS command interface.

## Milestone 1 — Complete IEC channels and DOS status handling

**Goal:** Establish the complete IEC communication foundation required by normal disk and file commands.

### IEC protocol and channels

- Complete secondary-address handling.
- Implement channel selection and channel state management.
- Implement the data channels required for normal file access.
- Implement the command channel using secondary address 15.
- Implement consistent channel open and close behavior.
- Handle channel switching, repeated operations and consecutive transfers.
- Ensure that failed transfers and commands leave the device in a valid state.

### DOS status and error channel

- Implement the DOS status/error channel.
- Define a central representation of DOS status codes and messages.
- Return the appropriate status after successful commands and file operations.
- Return meaningful errors for invalid commands and parameters.
- Handle missing images, missing files, invalid channels and invalid operations.
- Ensure status information can be read repeatedly and is updated at the appropriate points.

### DOS command foundation

- Introduce a dedicated DOS command parsing layer.
- Separate command parsing from IEC byte transfer and D64 sector storage.
- Define consistent command parameters and validation.
- Establish a common result/error path between DOS commands, drive operations and the status channel.

**Exit criterion:** Common IEC channel sequences work reliably. The command channel accepts commands, the status/error channel reports their results, and successful and failed operations leave the device in a valid state.

## Milestone 2 — Standard 1541 DOS commands and disk operations

**Goal:** Provide the normal externally visible DOS and drive functionality expected from a 1541-compatible disk drive, using D64 images.

This milestone follows directly after Milestone 1. The focus is the standard command interface, not the internal hardware implementation of a physical 1541.

### Sector and image operations

- Implement sector writing with track, sector and data-length validation.
- Validate sector accesses against the actual image geometry.
- Handle invalid and truncated images safely.
- Track image modifications consistently after writes.
- Persist modified images safely.
- Ensure that failed writes or saves do not silently discard modifications.

### BAM and directory management

- Implement allocation and release of disk sectors.
- Update BAM information after file and block operations.
- Implement directory-entry allocation and updates.
- Keep BAM and directory contents consistent after every operation.
- Handle full directories and disk-full conditions.
- Validate directory and sector chains, including termination and cycle detection.

### PRG file operations

- Complete and validate `LOAD` of PRG files through IEC from D64 images.
- Implement `SAVE` for PRG files.
- Implement file creation and directory-entry allocation.
- Implement file replacement.
- Implement file deletion and release allocated sectors.
- Handle file names, replacement syntax and access modes correctly.
- Return appropriate DOS errors for failed operations.

### Standard DOS command interface

Implement the normal DOS commands and operations exposed by a 1541-compatible drive, including:

- File channel operations such as `OPEN`, `CLOSE`, reading and writing.
- File operations such as `LOAD`, `SAVE`, replacement, rename and scratch.
- Disk initialization, validation and formatting.
- Command-channel status and error reporting.
- Block-level operations such as block read and block write.
- Buffer allocation and release behavior.
- Block positioning and buffer-pointer behavior.
- Disk identification and directory-related operations.
- Parameter validation and corresponding DOS error responses.

The command interface should be implemented according to the observable behavior of the standard 1541 DOS. Individual commands should be validated against VICE and representative C64 software.

Commands whose purpose is to access or execute arbitrary code inside the physical drive are not required. In particular, arbitrary drive RAM/ROM access and programming the internal 1541 CPU are outside the current target.

**Exit criterion:** Ordinary C64 software can use the emulated drive for standard file, disk and block operations. PRG files can be loaded, saved, replaced and deleted. Disk images can be formatted, modified, saved and reopened without corrupting their BAM or directory structures. DOS errors are reported through the status channel.

## Milestone 3 — Additional DOS file types and channel behavior

**Goal:** Extend file support beyond PRG files and complete less common standard DOS workflows.

### SEQ files

- Implement SEQ file reading.
- Implement SEQ file writing.
- Implement the corresponding DOS channel behavior.
- Validate sequential data transfer and file termination.
- Validate file creation, replacement and deletion.

### REL files

- Implement REL file reading.
- Implement REL file writing.
- Implement record-length handling.
- Implement side-sector allocation and management.
- Validate REL file behavior against VICE.

### Additional DOS behavior

- Verify wildcard and filename matching behavior.
- Verify file type and access-mode handling.
- Verify multiple open channels and channel reuse.
- Verify error handling for unsupported file types and invalid access modes.
- Verify command sequences involving multiple files and channels.

**Exit criterion:** Supported SEQ and REL operations behave consistently with the corresponding 1541 DOS semantics. Unsupported operations return meaningful errors rather than being silently treated as PRG operations.

## Deferred milestone — Disk directory GUI

**Goal:** Improve the usability of the disk device interface after the core IEC and DOS functionality is established.

The existing disk GUI already provides image management, activity indicators and Track/Sector information. A graphical directory view is useful, but it is not a prerequisite for the disk subsystem's core functionality.

### Shared directory representation

- Introduce a shared directory data model for the drive/storage layer and GUI.
- Keep directory parsing and interpretation in the storage/drive layer.
- Provide a consistent representation of filenames, file types, block counts, status flags and free blocks.
- Ensure the representation reflects the current in-memory image state.

### GUI integration

- Add a `DIR` action to the disk device GUI.
- Generate the directory on demand when the user requests it.
- Display filenames, file types, block counts and free blocks.
- Refresh the displayed directory after image insertion/ejection and after disk modifications.
- Provide useful error reporting for missing images and invalid directory data.

**Exit criterion:** The directory view always reflects the currently attached image and its current in-memory state. The GUI presents the directory data without duplicating D64 parsing logic.

## Milestone 4 — Define optional physical drive compatibility

**Goal:** Decide whether additional physical 1541 drive emulation is needed for concrete software compatibility requirements.

The current D64 sector-based approach is intended to support standard sector-oriented DOS functionality. It does not represent all physical disk characteristics.

Potential future extensions include:

- G64 and/or P64 image support.
- GCR encoding and decoding.
- Track and half-track representation.
- Drive CPU, RAM and ROM emulation.
- Two 6522 VIA instances and their drive-side behavior.
- Rotation, synchronization and other timing-dependent behavior.
- Copy-protection and non-standard disk layouts.
- Fastloaders and software that accesses drive hardware directly.

These features are not prerequisites for the standard DOS command interface. They should only be implemented if the intended compatibility level or concrete software requirements justify them.

In particular, arbitrary programming of the internal 1541 CPU or direct drive RAM/ROM access over IEC is not part of the current target.

**Exit criterion:** The project has a clear, evidence-based decision about whether any physical drive hardware emulation is needed beyond the standard D64/DOS implementation.

## Milestone 5 — Complete C64 hardware compatibility

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

## Milestone 6 — Memory expansion and additional devices

**Goal:** Support selected hardware expansions through a reusable architecture.

- Define and implement a reusable expansion-device interface.
- Implement GeoRAM.
- Implement REU registers, transfers and DMA interaction.
- Add persistent configuration and UI management for supported expansions.
- Add further IEC devices only when they serve a concrete compatibility or development requirement.

**Exit criterion:** Supported expansion devices can be configured, persisted and used by software through the emulated machine.

## Milestone 7 — Development-oriented emulator control

**Goal:** Provide stable interfaces for development tools.

- Synchronized machine-state access.
- Safe memory and CPU-register inspection.
- Controlled pause/resume and deterministic stepping.
- Breakpoints and execution until a specified condition.
- Disassembly and memory/register views.
- Program-loading and execution-control APIs.
- Symbol and address-mapping integration points.

**Exit criterion:** Development tools can inspect and control an emulated program through defined application interfaces.

## Milestone 8 — Integrated C64 development workflow

**Goal:** Turn C64Foundry into an integrated development environment.

- Project and workspace management.
- Source editor and file navigation.
- Assembler/compiler integration.
- Build diagnostics and PRG generation.
- One-step build-and-run in the integrated emulator.
- Symbol-aware and source-level debugging.

**Exit criterion:** A complete **Edit → Build → Run → Inspect → Fix** workflow works inside C64Foundry.

## Milestone 9 — Platform support and distribution

**Goal:** Improve usability and distribution without compromising the emulator architecture.

- Expand beyond Linux Wayland where maintainable.
- Improve packaging and release automation.
- Improve configuration and device-management workflows.
- Improve documentation and localization.
- Add regression tests for stable hardware, IEC and disk functionality.

## Guiding principles

1. **Standard drive functionality first:** Implement the normal IEC, DOS, file, disk and block commands expected by C64 software.
2. **No unnecessary hardware emulation:** Do not implement a complete physical 1541 merely to provide its standard DOS interface.
3. **VICE as a behavioral reference:** Use VICE to compare observable behavior and investigate edge cases, without blindly copying its architecture.
4. **Clear compatibility levels:** Standard D64/DOS compatibility is not the same as GCR, copy-protection, direct-drive programming, fastloader or cycle-exact drive compatibility.
5. **Separation of concerns:** Keep IEC protocol, DOS command handling, disk-image storage and GUI presentation in separate layers.
6. **Fresh state:** UI views such as the disk directory should be generated from the current image state, not stale cached output.
7. **Evidence-based status:** Distinguish implemented APIs, partial hardware behavior and complete user workflows.
8. **Incremental development:** Use buildable, testable steps and commit stable checkpoints.
9. **No speculative deadlines:** Reorder milestones as implementation discoveries and project requirements evolve.

For current capabilities see [README.md](README.md); for concrete implementation tasks see [TODO.md](TODO.md).

