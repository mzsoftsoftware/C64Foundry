# C64Foundry

C64Foundry is an early-stage, Qt 6/C++ Commodore 64 emulator with the long-term goal of becoming an integrated C64 software development environment.

The project combines an emulation core, a desktop control interface, video output, physical and virtual keyboard input, an IEC bus and disk/printer devices. The storage implementation is based on D64 disk images and is being extended toward comprehensive support for standard Commodore disk-drive commands and DOS operations.

> **Status:** Work in progress. C64Foundry is not yet a complete or cycle-exact C64 emulator. The disk subsystem is also still under development. Implemented components and partial functionality are distinguished from complete hardware compatibility and user workflows.

## Currently implemented

### Emulator core

- MOS 6510 CPU with a micro-operation/instruction-table implementation, documented and numerous undocumented operations, CPU port behavior, reset, IRQ and NMI handling, and bus-cycle support.
- 64 KiB RAM, BASIC/KERNAL/character ROM loading, color RAM, CPU-port-controlled memory banking and VIC-II memory access.
- CIA 6526 port and data-direction registers, Timer A and interrupt-mask/status handling. Other CIA functions remain incomplete.
- Two CIA instances, keyboard matrix input via CIA1, and RESTORE-driven NMI.
- VIC-II raster timing, raster IRQ, badline logic, BA/AEC bus arbitration, screen/character fetches, border behavior and a basic high-resolution character-pixel pipeline.
- Triple-buffered VIC-II frame data with a separate video worker that converts C64 color indices into Qt images.
- PAL, NTSC and old-NTSC timing definitions in the core. Runtime selection is not yet exposed as a user setting.
- Direct program-byte loading into RAM at a caller-supplied address via `C64Machine::loadProgram()`.

### Desktop application

- Qt Widgets main window with dockable emulator controls, video display and virtual C64 keyboard.
- Start, stop, reset, speed selection (0.5×, 1×, 2×, 5×, warp) and execution statistics.
- Saved and restored dock/window layout, including a layout-reset action.
- Physical keyboard input using native scan codes on **Linux Wayland**.
- Momentary and toggle mappings, multi-key mappings, physical/virtual input aggregation and key-state synchronization.
- Interactive virtual keyboard with mouse input, latched modifiers, key legends and visual pressed/hovered states.
- Configuration persistence for ROM paths, active configuration, platform identity and keyboard mapping overrides.
- Embedded OpenROMs as the default ROM set; optional detection of VICE ROM files at `/usr/share/vice/C64`.

### IEC bus and devices

- IEC bus and protocol implementation used for communication between the C64 and emulated IEC devices.
- IEC device manager supporting multiple devices and device numbers 8 through 11 for the disk implementation.
- ASCII printer device with printer output exposed in the desktop application.
- Foundry Disk device backed by D64 disk images.
- Disk image insertion, ejection and saving through the drive layer.
- D64 sector addressing and reading for standard 35-track disk images.
- Directory program generation from the disk BAM and directory sectors. The generated BASIC program is transferred through IEC and supports `LOAD "$",8`.
- Reading PRG files from a D64 image through the directory and file-sector chains.
- Filename normalization and basic prefix matching using a trailing `*`.
- Queued disk-sector access events, with separate read/write activity indicators in the disk device GUI.
- Track/Sector information displayed in the disk device GUI.

The disk implementation is sector-based and uses D64 images. Its goal is to provide the standard drive and DOS functionality expected by ordinary C64 software, without requiring a complete emulation of the physical 1541 drive hardware.

## Current limitations

### CPU and C64 hardware

- The VIC-II currently renders a basic character-pixel path. Bitmap/multicolor/extended-background modes and actual sprite rendering are missing. Several sprite-related registers can be read/written without corresponding sprite rendering.
- CIA Timer B, TOD, serial shift, alarm and other advanced behavior are not implemented.
- No SID sound synthesis or audio output.
- PAL/NTSC timing is not yet exposed as a complete user configuration workflow.

### IEC and disk drives

- The IEC disk device currently supports directory transfer and PRG file reading from D64 images.
- IEC secondary-address and channel handling still needs to be completed for the intended DOS functionality.
- The command channel and DOS status/error channel are not yet fully implemented.
- The standard DOS command set, including the required file and block operations, is not yet complete.
- Sector writing, `SAVE`, file creation, replacement, deletion and disk formatting are not yet implemented.
- SEQ and REL file handling is not yet implemented.
- The disk GUI already provides image management, drive activity indicators and Track/Sector information. An on-demand graphical directory view is planned but is not an immediate priority.
- The current implementation works with D64 sector images. G64/P64, GCR track data, half-tracks, copy-protection behavior and cycle-sensitive drive hardware are not implemented.
- Direct programming of the physical 1541 drive hardware over IEC, such as arbitrary drive memory access or execution of custom drive code, is outside the current compatibility target.

### Development environment and platform

- No integrated source editor, project manager, assembler/compiler pipeline or development debugger.
- Keyboard mappings for X11, Windows and macOS are not implemented; the application explicitly rejects unsupported keyboard platforms.
- Saved keyboard overrides exist in the data model but there is no complete configuration editor for them.
- ROM loading and machine timing are not yet presented as complete interactive configuration workflows.

## Requirements

- CMake 3.19 or newer
- C++ compiler supported by Qt 6
- Qt 6.5 or newer with Core, Widgets and LinguistTools
- Linux Wayland for the currently supported keyboard input backend

The application is built with strict compiler warnings (`-Wall -Wextra -Wpedantic -Werror`). Other operating systems are not currently supported as runnable keyboard-enabled targets.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build --parallel
./build/C64Foundry
```

The exact executable location may vary with the CMake generator and build configuration. No Docker setup is required.

## Getting started

1. Build and launch under a Linux Wayland session.
2. The application creates a default configuration with embedded OpenROMs on first run.
3. If a compatible VICE installation exists in the expected system directory, an additional ROM configuration can be detected.
4. Use the emulator control dock to start, stop or reset the machine and select its execution speed.
5. Use the physical keyboard or virtual C64 keyboard to send input; dock panels can be moved, hidden or floated.
6. Configure an IEC disk device and attach a supported D64 image to it.
7. From the emulated C64, use `LOAD "$",8` to load the generated disk directory or load a supported PRG file by its disk filename.

**Note:** The disk implementation is still under development. In particular, the DOS command channel, status/error handling, disk writing, file operations and formatting are not yet complete.

## Architecture

| Directory | Responsibility |
| --- | --- |
| `C64/CPU` | MOS 6510 execution and instruction micro-operations |
| `C64/Bus`, `C64/Memory` | Address decoding, banking and memory devices |
| `C64/CIA`, `C64/VIC-II`, `C64/Input` | C64 hardware and keyboard matrix |
| `C64/IEC` | IEC bus, protocol, devices, disk and drive logic |
| `Emulator` | Worker thread, command processing and speed regulation |
| `Input/Keyboard` | Host and virtual keyboard event aggregation |
| `Output/Video` | Frame acquisition and image conversion |
| `Gui` | Emulator controls, keyboard, video and device-related widgets |
| `Configuration` | Platform identification, ROM detection and persistent settings |
| `App` | Application initialization and main window |
| `Resources` | Embedded ROM resources and licenses |
| `Tools` | Development utilities |

The emulator core is a separate static library (`C64FoundryCore`). The Qt application owns the emulator and video workers and connects them to dockable widgets.

## Development direction

The immediate focus is completing the IEC disk subsystem so that ordinary C64 software can use the emulated drive through the standard IEC interface.

The next development steps are:

1. Complete IEC secondary-address and channel handling, including the command channel and DOS status/error channel.
2. Implement the standard DOS command interface and its error/status reporting.
3. Implement disk writing and the normal file operations, including `SAVE`, file creation, replacement and deletion.
4. Implement the standard block and disk-management commands, including the required BAM and directory updates.
5. Extend file support to additional DOS file types such as SEQ and REL.
6. Validate the observable behavior against VICE and real C64 BASIC commands and programs.

The intended compatibility target is a practical D64-based implementation of the standard 1541 DOS and drive command interface. This includes normal disk, file, channel and block operations.

It does **not** require emulating the internal 1541 hardware or allowing software to program the drive itself over the IEC bus. A separate drive CPU, VIA hardware, drive RAM/ROM, GCR processing, rotational timing, half-tracks and copy-protection support are not current priorities. Fastloaders or other software that depends on direct drive hardware access are not guaranteed to work.

The disk GUI's on-demand directory view and any additional shared directory presentation model are useful improvements, but they are deferred until the core IEC and DOS functionality is in place.

Longer-term goals include completing the C64 hardware implementation, memory expansions, development tools and an integrated source/build/run workflow. See [TODO.md](TODO.md) for actionable tasks and [ROADMAP.md](ROADMAP.md) for the planned development stages.

## Licensing

C64Foundry is distributed under the repository's [GPL-3.0 license](LICENSE). The bundled OpenROMs have their own license, included under `Resources/ROMs/OpenROMs/LICENSE`. Review both before redistributing binaries.

