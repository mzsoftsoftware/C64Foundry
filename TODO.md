# TODO — C64Foundry

This is the implementation-focused task list for the current repository state.

The list distinguishes between implemented functionality and work that is still required. An existing class, method or UI element does not by itself mean that the complete feature is finished.

- **Done:** Implemented and usable for the described scope.
- **Partial:** Some supporting implementation exists, but the feature is incomplete.
- **Open:** Not implemented yet.
- **Deferred:** Intentionally postponed; not part of the immediate development sequence.

The current priority is completing the IEC disk subsystem and the standard 1541 DOS/drive command interface using D64 images. Full emulation of the physical 1541 hardware is not the target. See [ROADMAP.md](ROADMAP.md) for the larger development stages and [README.md](README.md) for the current feature overview.

## P0 — IEC channels and DOS status/error handling

**Goal:** Establish the complete IEC communication foundation required by normal disk and file commands.

### Existing implementation

- [x] **Done:** IEC bus and protocol foundation.
- [x] **Done:** IEC device manager with multiple device instances.
- [x] **Done:** Foundry Disk device supports device numbers 8 through 11.
- [x] **Done:** Initial LISTEN/TALK handling for directory and PRG data transfer.
- [x] **Done:** Directory transfer for `LOAD "$",8`.
- [x] **Done:** PRG file reading from D64 directory and sector chains.
- [x] **Done:** Queued disk-sector access events and GUI read/write activity indication.
- [x] **Done:** Track/Sector information in the disk device GUI.

### IEC channels

- [ ] **Open:** Complete secondary-address handling.
- [ ] **Open:** Implement consistent channel selection and channel state management.
- [ ] **Open:** Implement the data channels required for normal file access.
- [ ] **Open:** Implement command channel handling using secondary address 15.
- [ ] **Open:** Implement consistent channel open and close behavior.
- [ ] **Open:** Handle channel state across repeated and consecutive operations.
- [ ] **Open:** Handle channel switching and cleanup after failed transfers.
- [ ] **Open:** Validate LISTEN/TALK and secondary-address sequencing against the expected IEC behavior.

### DOS status and error channel

- [ ] **Open:** Implement the DOS status/error channel.
- [ ] **Open:** Define a central representation of DOS status codes and status messages.
- [ ] **Open:** Return the appropriate status after successful commands and file operations.
- [ ] **Open:** Return meaningful errors for invalid commands and command parameters.
- [ ] **Open:** Handle missing images, missing files, invalid channels and invalid operations.
- [ ] **Open:** Ensure status information can be read repeatedly and is updated at the appropriate points.
- [ ] **Open:** Ensure errors do not leave the IEC device or its channels in an unusable state.

### DOS command parsing foundation

- [ ] **Open:** Implement a dedicated DOS command parsing layer.
- [ ] **Open:** Separate command parsing from IEC byte transfer and D64 sector storage.
- [ ] **Open:** Define command parameters and validation consistently.
- [ ] **Open:** Establish a common result/error path between DOS commands, drive operations and the status channel.

**Acceptance criteria:**

- Common IEC channel sequences work without relying on debug-only behavior.
- The command channel can receive and process commands.
- The status/error channel reports the result of commands and operations.
- Successful and failed operations leave the device in a valid state.
- The behavior can be tested using C64 BASIC commands and compared with VICE.

## P1 — Standard DOS commands and disk writing

**Goal:** Implement the normal 1541 DOS command interface and the disk operations required by ordinary C64 software.

This milestone follows directly after P0. It is not intended to emulate the internal 1541 hardware; it is intended to provide the standard externally visible DOS and drive functionality using D64 sectors.

### D64 sector operations

- [x] **Done:** D64 image insertion and ejection.
- [x] **Done:** Image data held in the drive layer.
- [x] **Done:** Image saving through the drive layer.
- [x] **Done:** Standard 35-track D64 sector addressing.
- [x] **Done:** Sector reading.
- [ ] **Open:** Implement sector writing with track, sector and data-length validation.
- [ ] **Open:** Validate all sector accesses against the actual image geometry.
- [ ] **Open:** Handle invalid or truncated images safely.
- [ ] **Open:** Track image modifications consistently after writes.
- [ ] **Open:** Ensure image saving persists the current in-memory image data.
- [ ] **Open:** Verify that failed writes or saves do not silently discard modifications.
- [ ] **Open:** Add robust handling for malformed BAM and directory structures.
- [ ] **Open:** Verify sector-chain termination and last-sector byte counts.
- [ ] **Open:** Check for invalid sector references and cycles in every relevant chain.

### BAM and directory management

- [x] **Done:** BAM reading from track 18, sector 0.
- [x] **Done:** Directory sector-chain traversal.
- [x] **Done:** Directory program generation from BAM and directory entries.
- [x] **Done:** Directory handling includes all eight entries in a directory sector.
- [x] **Done:** Directory chain cycle detection.
- [x] **Done:** Generate a BASIC directory program at load address `$0401`.
- [x] **Done:** Read disk name and DOS identification fields from the BAM.
- [x] **Done:** Generate directory entries with block counts, names, file types and status markers.
- [x] **Done:** Generate the free-block footer from BAM allocation information.
- [ ] **Open:** Implement directory-entry allocation and updates.
- [ ] **Open:** Implement BAM allocation and release of sectors.
- [ ] **Open:** Keep BAM and directory contents consistent after every operation.
- [ ] **Open:** Handle full directories and insufficient free directory entries.
- [ ] **Open:** Handle disk-full conditions and allocation failures.
- [ ] **Open:** Compare directory formatting and edge cases with VICE and a real C64 BASIC environment.

### PRG file operations

- [x] **Done:** PRG directory lookup and file-sector-chain reading.
- [x] **Done:** Detect cycles in the file-sector chain.
- [x] **Done:** Validate the final sector's used-byte count.
- [x] **Done:** Return PRG data including its two-byte load address.
- [ ] **Open:** Verify C64 BASIC `LOAD "PROGRAM",8` through IEC using a D64 image.
- [ ] **Open:** Implement `SAVE` for PRG files.
- [ ] **Open:** Implement file creation and directory-entry allocation.
- [ ] **Open:** Implement file replacement.
- [ ] **Open:** Implement file deletion and release allocated sectors.
- [ ] **Open:** Implement correct handling of file names and replacement syntax.
- [ ] **Open:** Handle disk-full conditions and insufficient free directory entries.
- [ ] **Open:** Validate file operations against VICE.

**Important distinction:** Loading a host PRG file through the application's GUI and loading a PRG file from a D64 image with the C64's IEC `LOAD` command are separate workflows. Host PRG loading is already implemented. The IEC/D64 `LOAD "PROGRAM",8` workflow still needs explicit validation.

### Standard DOS commands

- [ ] **Open:** Implement the standard DOS command parser and command dispatch.
- [ ] **Open:** Implement disk initialization behavior.
- [ ] **Open:** Implement disk validation behavior.
- [ ] **Open:** Implement disk formatting.
- [ ] **Open:** Implement file scratch/deletion.
- [ ] **Open:** Implement file rename.
- [ ] **Open:** Implement file copy where supported by the intended DOS behavior.
- [ ] **Open:** Implement the standard block read and block write commands.
- [ ] **Open:** Implement the required buffer allocation and release behavior.
- [ ] **Open:** Implement the required block positioning and buffer-pointer behavior.
- [ ] **Open:** Implement the standard disk identification and directory-related commands.
- [ ] **Open:** Implement command parameter validation and the corresponding DOS errors.
- [ ] **Open:** Verify that formatting creates a consistent BAM, directory sector and disk identification fields.
- [ ] **Open:** Verify that image changes can be saved and reopened without data loss.

### Standard command coverage

The intended command coverage includes the normal DOS interface exposed by a 1541-compatible drive, including:

- File channel operations such as `OPEN`, `CLOSE`, reading and writing.
- File operations such as `LOAD`, `SAVE`, replacement, rename and scratch.
- Disk initialization, validation and formatting.
- Command-channel status and error reporting.
- Block-level operations such as block read and block write.
- Buffer and block positioning operations required by the standard DOS interface.

Commands whose purpose is to access or execute arbitrary code inside the physical drive are not part of this target. In particular, arbitrary drive RAM/ROM access and programming the internal 1541 CPU are not required.

**Acceptance criteria:**

- Normal PRG files can be loaded, saved, replaced and deleted through the IEC interface.
- Standard disk and block commands work with supported D64 images.
- BAM and directory contents remain consistent after every operation.
- Formatting creates a usable disk image.
- DOS errors are reported correctly for common failure cases.
- Modified images can be saved and reopened without losing changes.

## P2 — Additional DOS file types and channel behavior

**Goal:** Extend file support beyond PRG files and complete less common standard DOS workflows.

### SEQ files

- [ ] **Open:** Implement SEQ file reading.
- [ ] **Open:** Implement SEQ file writing.
- [ ] **Open:** Implement the corresponding DOS channel behavior.
- [ ] **Open:** Validate sequential data transfer and file termination.
- [ ] **Open:** Validate file creation, replacement and deletion for SEQ files.

### REL files

- [ ] **Open:** Implement REL file reading.
- [ ] **Open:** Implement REL file writing.
- [ ] **Open:** Implement record-length handling.
- [ ] **Open:** Implement side-sector allocation and management.
- [ ] **Open:** Validate REL file behavior against VICE.

### Additional DOS behavior

- [ ] **Open:** Verify wildcard and filename matching behavior.
- [ ] **Open:** Verify file type and access-mode handling.
- [ ] **Open:** Verify multiple open channels and channel reuse.
- [ ] **Open:** Verify error handling for unsupported file types and invalid access modes.
- [ ] **Open:** Verify command sequences involving multiple files and channels.

**Acceptance criteria:**

- SEQ and REL files are handled according to their respective DOS semantics.
- Unsupported operations return meaningful errors instead of being treated as PRG operations.
- Multiple channels can be used without corrupting each other's state.

## Deferred — Disk GUI directory view and shared directory model

These tasks are useful for usability and code organization, but they are not prerequisites for the IEC/DOS implementation. They are intentionally postponed until the core drive functionality is in place.

### Directory data model

- [ ] **Deferred:** Introduce a shared directory representation for the drive/storage layer and GUI.
- [ ] **Deferred:** Keep directory parsing and interpretation in the storage/drive layer.
- [ ] **Deferred:** Provide a consistent representation of filenames, file types, block counts, status flags and free blocks.
- [ ] **Deferred:** Ensure the representation reflects the current in-memory image state.

### Directory GUI

- [ ] **Deferred:** Add a `DIR` action to the disk device GUI.
- [ ] **Deferred:** Generate the directory on demand when the user presses `DIR`.
- [ ] **Deferred:** Display filenames, file types, block counts and free blocks in a readable GUI view.
- [ ] **Deferred:** Refresh the displayed directory after image insertion/ejection and after disk modifications.
- [ ] **Deferred:** Show useful error information when directory generation fails.

**Acceptance criteria:**

- The directory is generated from the currently attached image and its current in-memory state.
- The GUI does not duplicate D64 parsing logic.
- The GUI handles an empty drive and invalid directory data gracefully.

## P3 — C64 hardware compatibility

### VIC-II

- [ ] **Partial:** Complete character display timing, character fetch/shift-register behavior, scrolling and border transitions.
- [ ] **Open:** Implement multicolor text mode.
- [ ] **Open:** Implement extended-background text mode.
- [ ] **Open:** Implement high-resolution bitmap mode.
- [ ] **Open:** Implement multicolor bitmap mode.
- [ ] **Partial:** Complete raster register and interrupt behavior, including acknowledge and timing details.
- [ ] **Partial:** Implement sprite data fetch and rendering.
- [ ] **Open:** Implement sprite positioning, expansion, multicolor and priority.
- [ ] **Open:** Implement sprite/sprite and sprite/background collisions and corresponding interrupts.
- [ ] **Partial:** Refine memory-fetch contention, badlines and PAL/NTSC raster timing.

### CIA and machine I/O

- [ ] **Partial:** Finish Timer A operating modes and underflow/interrupt corner cases.
- [ ] **Open:** Implement Timer B, its count modes and Timer A chaining.
- [ ] **Open:** Implement time-of-day clock and alarm registers.
- [ ] **Open:** Implement serial shift register behavior.
- [ ] **Open:** Implement CNT/FLAG input and PB6/PB7 timer outputs.
- [ ] **Partial:** Finish CIA interrupt behavior and port-level integration, including relevant CIA2 functions.
- [ ] **Open:** Define intentional SID-register behavior; sound synthesis remains a later feature.

## P4 — Configuration and host input

- [ ] **Partial:** Add UI to list, create, select, edit and delete persistent machine configurations.
- [ ] **Partial:** Add ROM-set selection and validation in the UI; VICE ROM detection currently uses a fixed Linux path.
- [ ] **Partial:** Expose PAL/NTSC selection and persist the chosen timing mode.
- [ ] **Partial:** Add a keyboard mapping editor using the existing override storage, including multiple C64 keys and momentary/toggle modes.
- [ ] **Partial:** Define safe input-state reset when switching mappings/configurations and when application focus changes.
- [ ] **Partial:** Complete the virtual keyboard's non-input/editor mode and mapping-preview workflow.
- [ ] **Open:** Add separate keyboard backends/mappings for X11, Windows and macOS only when those platforms become supported.
- [ ] **Partial:** Improve actionable startup feedback for invalid configurations and ROM-loading failures.

## P5 — Memory expansion and additional devices

- [ ] **Open:** Define a reusable expansion-device interface for IO1/IO2, cartridge signals and address-space integration.
- [ ] **Open:** Implement GeoRAM register/bank selection and expansion RAM.
- [ ] **Open:** Implement REU registers, transfers, DMA interaction and configurable memory capacity.
- [ ] **Open:** Add UI and persistent settings for expansion devices.
- [ ] **Open:** Consider additional IEC devices only when required by a concrete compatibility or development use case.

## P6 — Development-facing emulator API

- [ ] **Partial:** Introduce explicit pause/resume and synchronized machine-state access on top of the existing start/stop worker controls.
- [ ] **Partial:** Expose safe read/write memory operations, CPU registers and program loading through the emulator controller.
- [ ] **Open:** Add instruction stepping and controlled execution until a specified condition.
- [ ] **Open:** Add breakpoints.
- [ ] **Open:** Provide a minimal memory viewer, register viewer and disassembly interface.
- [ ] **Open:** Define source-address and symbol integration points for future tools.

## P7 — Integrated development environment

- [ ] **Open:** Implement project/workspace creation and persistence.
- [ ] **Open:** Add a source editor and file navigation.
- [ ] **Open:** Integrate assembler/compiler execution and collect diagnostics.
- [ ] **Open:** Build PRG artifacts and launch them directly in the emulator.
- [ ] **Open:** Add symbol-aware navigation, source-level breakpoints and source-to-address mapping.
- [ ] **Open:** Implement the complete Edit → Build → Run → Inspect workflow.

## Later — Drive hardware extensions, audio, platform support and distribution

The following are explicitly outside the immediate standard DOS compatibility target. They may be considered later if concrete software requirements justify them.

### Physical drive hardware compatibility

- [ ] **Deferred:** Evaluate G64 image support.
- [ ] **Deferred:** Evaluate P64 image support.
- [ ] **Deferred:** Evaluate GCR encoding and decoding.
- [ ] **Deferred:** Evaluate track and half-track representation.
- [ ] **Deferred:** Evaluate drive-side CPU, RAM and ROM emulation.
- [ ] **Deferred:** Evaluate the two 6522 VIA instances and their drive-side behavior.
- [ ] **Deferred:** Evaluate rotation, synchronization and timing-dependent behavior.
- [ ] **Deferred:** Evaluate copy-protection and non-standard disk layouts.
- [ ] **Deferred:** Evaluate fastloaders and software that accesses drive hardware directly.
- [ ] **Out of scope for current target:** Arbitrary programming of the internal 1541 CPU or direct drive RAM/ROM access over IEC.

### Audio, platform support and distribution

- [ ] **Open:** Add SID audio synthesis and host audio output.
- [ ] **Open:** Expand host-platform support after the Linux Wayland workflow is stable.
- [ ] **Open:** Improve packaging and release automation.
- [ ] **Open:** Improve configuration UX, localization and end-user documentation.
- [ ] **Open:** Add regression tests for stable CPU, VIC-II, CIA, IEC and disk functionality.

## Development and validation principles

1. Use VICE as the primary behavioral reference for standard IEC and 1541 DOS behavior.
2. Compare observable behavior rather than copying VICE's architecture.
3. Keep IEC protocol, DOS command handling, disk-image storage and GUI presentation separate.
4. Implement the standard drive command interface before considering physical drive hardware emulation.
5. Generate directory information from the current image state; do not rely on stale cached output.
6. Validate both successful operations and error cases.
7. Distinguish standard D64/DOS compatibility from GCR, copy-protection, direct-drive programming, fastloader and cycle-exact hardware compatibility.
8. Prefer coherent, buildable implementation steps and commit stable checkpoints.
9. Do not mark a feature complete until its behavior is tested in the intended workflow.

