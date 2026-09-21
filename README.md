# C64Foundry

C64Foundry is a C64 development environment with an integrated C64 emulator, editor and debugger.

The project is developed in C++ and Qt 6 and is intended to provide an integrated development environment specifically for the Commodore 64.

## Status

**Early development**

C64Foundry is currently in the initial development phase. The project structure and basic application foundation are being established before the C64 emulation core is implemented.

## Goals

C64Foundry is intended to combine the tools needed for C64 development in a single application:

* C64 emulator
* C64 monitor
* source code editor
* debugger
* disassembler
* assembler/compiler integration
* project management
* disk image support
* C64 hardware and memory inspection

The emulator will be implemented specifically for C64 hardware and will not depend on SDL or other multimedia frameworks.

## Architecture

The project is designed to separate the C64 emulation core from the Qt-based application and user interface.

The planned high-level architecture is:

```text
C64Foundry
├── Core
│   ├── CPU 6510
│   ├── Memory
│   ├── VIC-II
│   ├── CIA
│   ├── SID
│   ├── IEC
│   ├── Disk Drive 1541
│   └── C64
│
├── Emulator
│
├── Debugger
│
└── GUI
    ├── Main Window
    ├── Project
    ├── Editor
    ├── C64 Monitor
    ├── Debugger
    ├── Registers
    ├── Memory
    ├── Disassembler
    └── Output
```

The emulation core is intended to remain independent of Qt wherever practical. Qt 6 is used for the application, graphical user interface, input handling and audio/video output.

## C64 Emulator

The emulator is an independent implementation.

Its development is informed by publicly available C64 hardware and software documentation and by studying existing C64 emulators and related projects, including [VICE](https://vice-emu.sourceforge.io/) and [Emu64](https://github.com/ThKattan/emu64).

No source code from these projects is intended to be used in C64Foundry.

Technical references are documented in [docs/REFERENCES.md](docs/REFERENCES.md).

## Technology

* C++
* C++20
* Qt 6
* CMake
* Native Qt Widgets
* No SDL / SDL2 dependency

## Building

C64Foundry uses CMake and requires Qt 6.

A typical build can be performed with:

```bash
cmake -S . -B build
cmake --build build
```

The project is primarily developed and tested on Linux.

## Development

C64Foundry is developed as a modular project. The emulator core, debugger and graphical user interface are kept as separate areas of the codebase to make the emulator usable independently of the GUI where practical.

Development decisions and architectural information are documented in the `docs` directory.

## License

C64Foundry is licensed under the Apache License 2.0.

See the [LICENSE](LICENSE) file for the full license text.

Copyright © 2026 MZ Software GmbH.
