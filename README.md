# C64Foundry

C64Foundry is an early-stage, Qt 6/C++ Commodore 64 emulator intended to become an integrated C64 software development environment. The project currently provides an emulator core, a desktop control interface, video output and host/virtual keyboard input. An editor, assembler/compiler integration, file-device support and development debugger are not yet available.

> **Status:** Work in progress. This is not a complete or cycle-perfect C64 emulator. Functionality described below is based on the implementation in the supplied repository snapshot, not a claim of general software compatibility.

## Currently implemented

### Emulator core

- MOS 6510 CPU with a micro-operation/instruction-table implementation, documented and numerous undocumented operations, CPU port behavior, reset, IRQ and NMI handling, and bus-cycle support.
- 64 KiB RAM, BASIC/KERNAL/character ROM loading, color RAM, CPU-port-controlled memory banking and VIC-II memory access.
- CIA 6526 port and data-direction registers, Timer A and interrupt-mask/status handling. Other CIA functions remain incomplete.
- Two CIA instances, keyboard matrix input via CIA1, and RESTORE-driven NMI.
- VIC-II raster timing, raster IRQ, badline logic, BA/AEC bus arbitration, screen/character fetches, border behavior and a basic high-resolution character-pixel pipeline.
- Triple-buffered VIC-II frame data with a separate video worker that converts C64 color indices into Qt images.
- PAL, NTSC and old-NTSC timing definitions in the core. Runtime selection is not yet exposed as a user setting.
- Direct program-byte loading into RAM at a caller-supplied address via `C64Machine::loadProgram()`. This is **not** a PRG-file loader.

### Desktop application

- Qt Widgets main window with dockable emulator controls, video display and virtual C64 keyboard.
- Start, stop, reset, speed selection (0.5×, 1×, 2×, 5×, warp) and execution statistics.
- Saved and restored dock/window layout, including a layout-reset action.
- Physical keyboard input using native scan codes on **Linux Wayland**.
- Momentary and toggle mappings, multi-key mappings, physical/virtual input aggregation and key-state synchronization.
- Interactive virtual keyboard with mouse input, latched modifiers, key legends and visual pressed/hovered states.
- Configuration persistence for ROM paths, active configuration, platform identity and keyboard mapping overrides.
- Embedded OpenROMs as the default ROM set; optional detection of VICE ROM files at `/usr/share/vice/C64`.

### Important limitations

- The VIC-II currently renders a basic character-pixel path; bitmap/multicolor/extended-background modes and actual sprite rendering are missing. Several sprite-related registers can be read/written without corresponding sprite rendering.
- CIA Timer B, TOD, serial shift, alarm and other advanced behavior are not implemented.
- No SID sound synthesis or audio output.
- No `.prg` file import workflow, IEC device, 1541 drive, disk-image handling, GeoRAM or REU.
- No integrated source editor, project manager, assembler/compiler pipeline or debugger.
- Keyboard mappings for X11, Windows and macOS are not implemented; the application explicitly rejects unsupported keyboard platforms.
- Saved keyboard overrides exist in the data model but there is no configuration editor for them.
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

**Note:** The application does not yet provide a GUI to switch between stored machine configurations or load a PRG file.

## Architecture

| Directory | Responsibility |
| --- | --- |
| `C64/CPU` | MOS 6510 execution and instruction micro-operations |
| `C64/Bus`, `C64/Memory` | Address decoding, banking and memory devices |
| `C64/CIA`, `C64/VIC-II`, `C64/Input` | C64 hardware and keyboard matrix |
| `Emulator` | Worker thread, command processing and speed regulation |
| `Input/Keyboard` | Host and virtual keyboard event aggregation |
| `Output/Video` | Frame acquisition and image conversion |
| `Gui` | Emulator controls, keyboard and video widgets |
| `Configuration` | Platform identification, ROM detection and persistent settings |
| `App` | Application initialization and main window |
| `Resources` | Embedded ROM resources and licenses |
| `Tools` | Development utilities |

The emulator core is a separate static library (`C64FoundryCore`). The Qt application owns the emulator and video workers and connects them to dockable widgets.

## Development direction

The next focus is practical program loading and completing missing emulator functionality required by C64 software. Expansion memory, storage access, project/editor integration and development-oriented debugging follow in stages. See [TODO.md](TODO.md) for actionable work and [ROADMAP.md](ROADMAP.md) for milestones.

## Licensing

C64Foundry is distributed under the repository's [GPL-3.0 license](LICENSE). The bundled OpenROMs have their own license, included under `Resources/ROMs/OpenROMs/LICENSE`. Review both before redistributing binaries.
