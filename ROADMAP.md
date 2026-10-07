# C64Foundry Roadmap

C64Foundry is intended to become an integrated development environment for developing, testing, and running software for the Commodore 64.

The goal is **not** to implement every detail of C64 hardware emulation before development tools can be built. Instead, the emulator should evolve according to the requirements of real C64 programs and the C64Foundry development workflow.

The general development strategy is therefore:

> Build enough accurate C64 emulation to support real-world development, use real C64 test programs to identify compatibility gaps, and prioritize features that directly improve the C64Foundry development workflow.

---

## 1. Complete Keyboard Input

**Priority: Immediate**

Finish the host-to-C64 keyboard input layer.

Current keyboard mappings already support multiple C64 keys and distinguish between momentary and toggle mappings, but the input controller does not yet fully implement these capabilities.

### Tasks

- Implement toggle key state handling.
- Implement `Shift Lock` correctly.
- Support mappings containing multiple simultaneous C64 keys.
- Correctly generate press/release events for multi-key mappings.
- Reset active/toggled states safely when configurations change.
- Add keyboard mappings for additional host platforms:
  - Linux X11
  - Windows
  - macOS
- Implement user-defined keyboard mapping overrides.
- Persist keyboard configuration.

---

## 2. PRG Loading and Test Program Infrastructure

**Priority: Very High**

Make it possible to load and execute real C64 programs.

This is the next major milestone after keyboard input.

### Tasks

- Load `.prg` files into C64 memory.
- Interpret the PRG load address.
- Support different execution strategies where useful:
  - BASIC program loading
  - KERNAL-compatible loading
  - Direct execution at a specified address
- Provide a simple application workflow for loading and starting programs.
- Establish a reproducible mechanism for running C64 hardware test programs.

### Milestone

**C64Foundry can load and execute real C64 test programs.**

---

## 3. Validate the Emulator with Real C64 Test Programs

**Priority: Very High**

Before completing individual chips simply for completeness, use established C64 test programs to determine which hardware behavior is actually missing or incorrect.

Test areas should include:

- MOS 6510
- Memory mapping
- CPU port / PLA behavior
- Interrupts
- CIA
- VIC-II
- Raster timing
- Bus timing
- KERNAL compatibility

Test results should drive subsequent emulator development.

Where possible, tests with clear PASS/FAIL results should be preferred.

Real C64 software and C64Foundry-developed programs should additionally be used as integration tests.

---

## 4. Program and File Handling

**Priority: High**

Provide the basic infrastructure required to work conveniently with C64 software.

### Tasks

- Load PRG files.
- Save/export PRG files where appropriate.
- Establish C64 file abstractions required by the development environment.
- Prepare device/file handling for later IEC integration.
- Integrate loading into the normal C64Foundry workflow.

The initial goal is convenient software development rather than exact disk-drive emulation.

---

## 5. Expansion Port, GeoRAM and REU

**Priority: High**

Expansion hardware is important for software intended to be developed with C64Foundry.

### Expansion infrastructure

Implement the necessary C64 expansion-port architecture, including:

- GAME / EXROM handling
- IO1 (`$DE00`)
- IO2 (`$DF00`)
- ROML / ROMH where required
- Cartridge/expansion memory mapping
- Extensible expansion-device interface

### GeoRAM

Implement GeoRAM support early because of its relatively straightforward memory banking model.

### REU

Implement Commodore REU support, including the required register interface and DMA behavior.

The expansion architecture should allow additional cartridge and memory devices to be added later without modifying the core bus architecture unnecessarily.

---

## 6. Emulator Control and Development API

**Priority: High**

Prepare the emulator core for integration with development tools.

### Tasks

- Reliable start/stop/reset behavior.
- Defined power-on and reset state.
- PAL/NTSC machine configuration.
- Controlled pause/resume.
- Safe access to machine state while paused.
- CPU state access.
- Memory access suitable for development tools.
- Program loading interface.
- Infrastructure for future breakpoints and stepping.

This layer should separate development tools from the internal implementation details of the emulator.

---

## 7. Extend CIA and VIC-II According to Actual Requirements

**Priority: Demand-driven**

CIA and VIC-II should not initially be completed merely for the sake of hardware completeness.

Missing functionality should be prioritized based on:

1. C64 hardware test failures.
2. Requirements of C64Foundry.
3. Requirements of programs developed with C64Foundry.
4. Compatibility problems with real C64 software.

### Potential CIA work

Examples include:

- Timer B
- Timer chaining/count modes
- TOD clock
- Alarm
- Serial shift register
- CNT/FLAG behavior
- PB6/PB7 timer outputs
- Additional CRA/CRB behavior

### Potential VIC-II work

Examples include:

- Fine scrolling
- Multicolor text mode
- Extended background color mode
- HiRes bitmap mode
- Multicolor bitmap mode
- Sprites
- Sprite expansion
- Sprite multicolor
- Sprite/background priority
- Sprite collisions
- Additional VIC-II interrupt sources
- More accurate memory-fetch and bus timing

Implementation order should be determined by actual test and software requirements.

---

## 8. IEC and Storage Device Support

**Priority: High**

C64Foundry needs practical access to files and storage devices used by C64 software.

The first implementation does **not** need to be a cycle-exact Commodore 1541 emulator.

### Initial goal

Provide an IEC/device implementation comparable in purpose to devices such as SD2IEC:

- LOAD
- SAVE
- Directory access
- Device numbers
- Basic IEC communication required by C64 software

### Later

If required by software compatibility:

- More accurate IEC timing
- Disk image support
- D64 handling
- Full 1541 emulation
- 6502/VIA emulation
- GCR and drive mechanics

A full 1541 implementation should only be undertaken when its additional compatibility is actually required.

---

## 9. Configuration and Essential Emulator UI

**Priority: Medium/High**

Complete configuration management for features that directly affect development.

### Configuration

- ROM sets
- PAL/NTSC
- Keyboard mappings
- Storage/device configuration
- Expansion devices
- GeoRAM
- REU
- Relevant emulator settings

Configurations should be persistent.

### Emulator UI

Only functionality useful to the development workflow should initially be prioritized.

Examples:

- Load/run program
- Reset
- Start/stop
- Speed control
- Machine configuration
- Expansion configuration
- Storage configuration

Pure emulator convenience features can remain lower priority.

---

## 10. C64Foundry Development Environment

**Priority: Very High once the emulator foundation is sufficient**

This is the primary long-term purpose of the project.

The development environment should gradually provide:

- Project/workspace management
- Source code editor
- Build system
- Assembler integration
- Compiler integration
- PRG generation
- Launch program directly in the integrated emulator
- Build diagnostics
- Error navigation
- Symbol information
- Source-address mapping
- Integration between source code and emulator state

The objective is a workflow similar to:

**Edit → Build → Run → Inspect → Fix**

without leaving C64Foundry.

---

## 11. Development-Oriented Debugging

**Priority: Medium**

A complete traditional machine monitor is not an immediate project goal.

Debugging functionality should primarily support software development inside C64Foundry.

Useful features include:

- Breakpoints
- CPU registers
- Memory viewer
- Disassembly
- Single instruction stepping
- Step over
- Step out
- Symbol-aware debugging
- Source-level breakpoint mapping
- Source-level execution tracking

More advanced monitor functionality can be added later if it proves useful.

---

## 12. SID and Audio

**Priority: Low**

SID emulation is intentionally deferred.

Audio is currently not required by the primary software development use cases for C64Foundry.

Until SID functionality becomes necessary, the SID address range may remain minimally implemented or stubbed.

Future work may include:

- SID registers
- Oscillators
- ADSR envelopes
- Waveforms
- Filters
- Audio mixing
- Host audio output

SID work should be prioritized when software developed or tested with C64Foundry actually requires it.

---

## 13. Documentation and Testing Policy

### Documentation

The repository documentation should be updated once the emulator has reached a stable and useful development milestone.

Documentation should eventually describe:

- Project goals
- Architecture
- Emulator capabilities
- Known limitations
- Supported hardware
- Development workflow
- Build instructions
- External references

The README should reflect the actual implementation state rather than an early project plan.

### Testing policy

Automated unit tests are primarily intended for **core components**.

Examples:

- CPU
- Memory
- Bus
- CIA
- VIC-II
- Expansion hardware
- Other deterministic emulator-core components

GUI and application-level components do not require unit tests by default.

Real C64 hardware test programs and real software should complement the core unit tests as integration and compatibility tests.

---

# Current Development Order

The current planned order is:

1. **Keyboard input**
2. **PRG loader**
3. **Real C64 test programs**
4. **Fix emulator deficiencies discovered by those tests**
5. **Program/file handling**
6. **Expansion architecture**
7. **GeoRAM**
8. **REU**
9. **Emulator development API**
10. **IEC / practical storage-device support**
11. **Configuration and essential emulator UI**
12. **C64Foundry editor/project/build environment**
13. **Development-oriented debugging**
14. **Additional CIA/VIC-II accuracy as required**
15. **SID/audio when actually required**

This order is intentionally flexible.

**Real software requirements and reproducible hardware test failures take precedence over implementing hardware features merely for completeness.**
