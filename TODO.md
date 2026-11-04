# TODO — C64Foundry

This is the implementation-focused task list for the current repository state.

The list distinguishes between implemented functionality and work that is still required. An existing class, method or UI element does not by itself mean that the complete feature is finished.

- **Done:** Implemented and usable for the described scope.
- **Partial:** Some supporting implementation exists, but the feature is incomplete.
- **Open:** Not implemented yet.

The current priority is the IEC disk subsystem and compatibility with a Commodore 1541/1541-II. See [ROADMAP.md](ROADMAP.md) for the larger development stages and [README.md](README.md) for the current feature overview.

## P0 — Disk GUI integration

### Disk status and directory view

- [ ] **Open:** Display the current Track and Sector information for the selected disk device.
- [ ] **Open:** Add a `DIR` action to the disk device GUI.
- [ ] **Open:** Generate the directory on demand when the user presses `DIR`.
- [ ] **Open:** Generate the directory from the currently attached image and its current in-memory state.
- [ ] **Open:** Display filenames, file types, block counts and free blocks in a readable GUI view.
- [ ] **Open:** Refresh the displayed directory after image insertion/ejection and after later disk modifications.
- [ ] **Open:** Show useful error information when directory generation fails.
- [ ] **Open:** Keep D64 parsing and directory generation in the drive/storage layer; do not duplicate this logic in the GUI.

**Acceptance criteria:**
- The directory is not a stale copy from an earlier image or an earlier state.
- Pressing `DIR` generates the listing from the currently selected disk image.
- The GUI handles an empty drive and invalid directory data gracefully.

## P1 — Complete the 1541/1541-II IEC protocol

### Existing implementation

- [x] **Done:** IEC bus and protocol foundation.
- [x] **Done:** IEC device manager with multiple device instances.
- [x] **Done:** Foundry Disk device supports device numbers 8 through 11.
- [x] **Done:** Initial LISTEN/TALK handling for directory and PRG data transfer.
- [x] **Done:** Directory transfer for `LOAD "$",8`.
- [x] **Done:** PRG file reading from D64 directory and sector chains.
- [ ] **Partial:** Complete IEC protocol behavior and channel handling.
- [ ] **Partial:** Command channel and DOS status/error channel are not implemented completely.

### IEC channels and commands

- [ ] **Open:** Complete secondary-address handling.
- [ ] **Open:** Implement command channel handling (secondary address 15).
- [ ] **Open:** Implement the status/error channel.
- [ ] **Open:** Implement consistent channel open and close behavior.
- [ ] **Open:** Handle channel state and command sequencing correctly.
- [ ] **Open:** Implement the DOS command parsing layer.
- [ ] **Open:** Return the appropriate DOS status codes and error messages.
- [ ] **Open:** Handle invalid commands, invalid channels, missing images and missing files.
- [ ] **Open:** Validate behavior for repeated commands and consecutive transfers.
- [ ] **Open:** Compare the observable IEC behavior with VICE.

**Acceptance criteria:**
- Common IEC command sequences work without relying on debug-only behavior.
- Errors are reported through the expected DOS status mechanism.
- The device returns to a valid state after successful and failed operations.

## P2 — Complete D64 sector and directory handling

### Existing implementation

- [x] **Done:** D64 image insertion and ejection.
- [x] **Done:** Image data held in the drive layer.
- [x] **Done:** Image saving through the drive layer.
- [x] **Done:** Standard 35-track D64 sector addressing.
- [x] **Done:** Sector reading.
- [x] **Done:** BAM reading from track 18, sector 0.
- [x] **Done:** Directory sector-chain traversal.
- [x] **Done:** Directory program generation from BAM and directory entries.
- [x] **Done:** Directory handling includes all eight entries in a directory sector.
- [x] **Done:** Directory chain cycle detection.
- [x] **Done:** PRG directory lookup and file-sector-chain reading.
- [ ] **Open:** Sector writing is currently not implemented.

### Sector access and validation

- [ ] **Open:** Implement sector writing with track, sector and data-length validation.
- [ ] **Open:** Validate all sector accesses against the actual image geometry.
- [ ] **Open:** Handle invalid or truncated images safely.
- [ ] **Open:** Track image modifications consistently after writes.
- [ ] **Open:** Ensure that image saving persists the current in-memory image data.
- [ ] **Open:** Verify that failed writes or saves do not silently discard modifications.
- [ ] **Open:** Add robust handling for malformed BAM and directory structures.
- [ ] **Open:** Verify sector-chain termination and last-sector byte counts.
- [ ] **Open:** Check for invalid sector references and cycles in every relevant chain.

### Directory behavior

- [x] **Done:** Generate a BASIC directory program at load address `$0401`.
- [x] **Done:** Read disk name and DOS identification fields from the BAM.
- [x] **Done:** Generate directory entries with block counts, names, file types and status markers.
- [x] **Done:** Generate the free-block footer from BAM allocation information.
- [ ] **Open:** Compare directory formatting and edge cases with VICE and a real C64 BASIC environment.
- [ ] **Open:** Ensure directory display and IEC directory transfer use the same underlying directory data.
- [ ] **Open:** Verify behavior for full directories and multiple directory sectors.

## P3 — Standard 1541 DOS file operations

### PRG files

- [x] **Done:** Find PRG entries in the D64 directory.
- [x] **Done:** Read PRG data from a linked sequence of sectors.
- [x] **Done:** Detect cycles in the file-sector chain.
- [x] **Done:** Validate the final sector's used-byte count.
- [x] **Done:** Return PRG data including its two-byte load address.
- [ ] **Open:** Implement `SAVE` for PRG files.
- [ ] **Open:** Implement file creation and directory-entry allocation.
- [ ] **Open:** Implement file replacement.
- [ ] **Open:** Implement file deletion and release allocated sectors.
- [ ] **Open:** Update BAM allocation information after file operations.
- [ ] **Open:** Handle disk-full conditions and insufficient free directory entries.
- [ ] **Open:** Validate file operations against VICE.

### Disk commands

- [ ] **Open:** Implement the standard 1541 DOS commands needed for normal disk use.
- [ ] **Open:** Implement disk initialization behavior.
- [ ] **Open:** Implement disk validation behavior.
- [ ] **Open:** Implement disk formatting.
- [ ] **Open:** Implement the required error handling for invalid command parameters.
- [ ] **Open:** Verify that formatting creates a consistent BAM, directory sector and disk identification fields.
- [ ] **Open:** Verify that image changes can be saved and reopened without data loss.

**Acceptance criteria:**
- PRG files can be loaded, saved, replaced and deleted.
- BAM and directory contents remain consistent after every operation.
- Formatting creates a usable disk image.
- DOS errors are reported correctly for common failure cases.

## P4 — SEQ and REL files

### SEQ files

- [ ] **Open:** Implement SEQ file reading.
- [ ] **Open:** Implement SEQ file writing.
- [ ] **Open:** Implement the corresponding DOS channel behavior.
- [ ] **Open:** Validate sequential data transfer and file termination.

### REL files

- [ ] **Open:** Implement REL file reading.
- [ ] **Open:** Implement REL file writing.
- [ ] **Open:** Implement record-length handling.
- [ ] **Open:** Implement side-sector allocation and management.
- [ ] **Open:** Validate REL file behavior against VICE.

**Acceptance criteria:**
- SEQ and REL files are handled according to their respective 1541 DOS semantics.
- Unsupported operations return meaningful errors instead of being treated as PRG operations.

## P5 — Define the required drive hardware compatibility

The current implementation uses D64 sector images. This is sufficient as a foundation for standard sector-oriented disk DOS behavior, but it does not represent the complete physical 1541 drive.

- [ ] **Open:** Define the intended compatibility level for 1541/1541-II software.
- [ ] **Open:** Identify which standard DOS operations can be completed with the current sector-based architecture.
- [ ] **Open:** Evaluate G64 image support.
- [ ] **Open:** Evaluate P64 image support.
- [ ] **Open:** Evaluate GCR encoding and decoding.
- [ ] **Open:** Evaluate track and half-track representation.
- [ ] **Open:** Evaluate drive-side CPU, RAM and ROM emulation.
- [ ] **Open:** Evaluate the two 6522 VIA instances and their drive-side behavior.
- [ ] **Open:** Evaluate rotation, synchronization and timing-dependent behavior.
- [ ] **Open:** Evaluate copy-protection and non-standard disk layouts.
- [ ] **Open:** Evaluate fastloaders and software that accesses drive hardware directly.

**Priority note:** Do not implement a complete cycle-exact 1541 merely because VICE does so. First complete standard DOS compatibility using D64 sector images. Add drive hardware emulation when the required compatibility target or actual software makes it necessary.

## P6 — C64 hardware compatibility

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

## P7 — Configuration and host input

- [ ] **Partial:** Add UI to list, create, select, edit and delete persistent machine configurations.
- [ ] **Partial:** Add ROM-set selection and validation in the UI; VICE ROM detection currently uses a fixed Linux path.
- [ ] **Partial:** Expose PAL/NTSC selection and persist the chosen timing mode.
- [ ] **Partial:** Add a keyboard mapping editor using the existing override storage, including multiple C64 keys and momentary/toggle modes.
- [ ] **Partial:** Define safe input-state reset when switching mappings/configurations and when application focus changes.
- [ ] **Partial:** Complete the virtual keyboard's non-input/editor mode and mapping-preview workflow.
- [ ] **Open:** Add separate keyboard backends/mappings for X11, Windows and macOS only when those platforms become supported.
- [ ] **Partial:** Improve actionable startup feedback for invalid configurations and ROM-loading failures.

## P8 — Memory expansion and additional devices

- [ ] **Open:** Define a reusable expansion-device interface for IO1/IO2, cartridge signals and address-space integration.
- [ ] **Open:** Implement GeoRAM register/bank selection and expansion RAM.
- [ ] **Open:** Implement REU registers, transfers, DMA interaction and configurable memory capacity.
- [ ] **Open:** Add UI and persistent settings for expansion devices.
- [ ] **Open:** Consider additional IEC devices only when required by a concrete compatibility or development use case.

SD2IEC-specific extensions are not a priority unless they are also needed for the intended 1541/1541-II compatibility.

## P9 — Development-facing emulator API

- [ ] **Partial:** Introduce explicit pause/resume and synchronized machine-state access on top of the existing start/stop worker controls.
- [ ] **Partial:** Expose safe read/write memory operations, CPU registers and program loading through the emulator controller.
- [ ] **Open:** Add instruction stepping and controlled execution until a specified condition.
- [ ] **Open:** Add breakpoints.
- [ ] **Open:** Provide a minimal memory viewer, register viewer and disassembly interface.
- [ ] **Open:** Define source-address and symbol integration points for future tools.

## P10 — Integrated development environment

- [ ] **Open:** Implement project/workspace creation and persistence.
- [ ] **Open:** Add a source editor and file navigation.
- [ ] **Open:** Integrate assembler/compiler execution and collect diagnostics.
- [ ] **Open:** Build PRG artifacts and launch them directly in the emulator.
- [ ] **Open:** Add symbol-aware navigation, source-level breakpoints and source-to-address mapping.
- [ ] **Open:** Implement the complete Edit → Build → Run → Inspect workflow.

## Later — Audio, platform support and distribution

- [ ] **Open:** Add SID audio synthesis and host audio output.
- [ ] **Open:** Expand host-platform support after the Linux Wayland workflow is stable.
- [ ] **Open:** Improve packaging and release automation.
- [ ] **Open:** Improve configuration UX, localization and end-user documentation.
- [ ] **Open:** Add regression tests for stable CPU, VIC-II, CIA, IEC and disk functionality.

## Development and validation principles

1. Use VICE as the primary behavioral reference for 1541/1541-II DOS and IEC compatibility.
2. Compare observable behavior rather than copying VICE's architecture without a concrete reason.
3. Keep IEC protocol, DOS behavior, disk-image storage and GUI presentation separate.
4. Generate directory information from the current image state; do not rely on stale cached output.
5. Validate both successful operations and error cases.
6. Distinguish standard D64/DOS compatibility from GCR, copy-protection, fastloader and cycle-exact drive compatibility.
7. Prefer coherent, buildable implementation steps and commit stable checkpoints.
8. Do not mark a feature complete until its behavior is tested in the intended workflow.

