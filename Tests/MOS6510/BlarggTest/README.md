# Blargg NES CPU Instruction Tests

Source:
Shay Green (blargg)

Test suite:
NES CPU Instruction Behavior Tests
instr_test-v5

Upstream repository:
https://github.com/christopherpow/nes-test-roms

Files:

* all_instrs.nes
* 03-immediate.nes

The original upstream readme is included unchanged as:

* readme.txt

## Purpose

The Blargg instr_test-v5 suite verifies 6502 instruction behavior,
including many unofficial NMOS 6502 instructions.

The test programs run on the emulated CPU itself. They initialize CPU
state and memory, execute instructions for many combinations of
registers, flags, operands, and memory values, calculate running
checksums from the resulting state, and compare those checksums with
known correct values.

This makes the suite an independent self-checking functional test,
similar in purpose to the Dormann and Seddon tests already used by
C64Foundry.

## Initial Integration

The initial C64Foundry integration uses:

* 03-immediate.nes

The single-test ROM is used first because it contains a standard
32 KiB PRG image and does not require the bank switching used by
all_instrs.nes.

The combined all_instrs.nes ROM is retained for possible later
integration.

## ROM Format

The Blargg test ROMs use the iNES file format.

The 16-byte iNES header is not part of the CPU address space.

03-immediate.nes contains two 16 KiB PRG banks, giving a total of
32 KiB PRG data.

The PRG data is mapped to:

$8000-$FFFF

The CPU reset vector at:

$FFFC-$FFFD

is therefore contained in the PRG image.

C64Foundry extracts the PRG image from the iNES file, copies it into
the test memory at $8000-$FFFF, reads the reset vector, and starts the
MOS6510 at the address specified by that vector.

## Test Result Interface

Blargg provides a memory-based result interface beginning at $6000.

$6000:

* $80 = test running
* $81 = reset requested
* $00-$7F = test finished with this result code
* $00 = passed

Text output starts at $6004 and is zero terminated.

C64Foundry monitors this interface while executing the test.

This allows the test result to be obtained without relying on the NES
video output.

## 03-immediate.nes

The initial test ROM verifies immediate addressing instructions.

The tested instructions include documented instructions such as:

* LDA
* LDX
* LDY
* ADC
* SBC
* ORA
* AND
* EOR
* CMP
* CPX
* CPY

It also includes several unofficial instructions, including:

* $EB SBC
* undocumented immediate NOP opcodes
* $0B/$2B ANC
* $4B ASR/ALR
* $6B ARR
* $AB ATX
* $CB AXS

At the time the Blargg test was added to C64Foundry, all of these
instructions except $AB and $CB had already been implemented and
tested independently.

This makes 03-immediate.nes useful as the first Blargg integration
test because it should exercise a substantial amount of the existing
MOS6510 implementation before reaching one of the remaining
unimplemented undocumented instructions.

## NES Hardware Dependencies

The Blargg ROMs were designed to run on NES hardware and their common
test shell performs accesses to NES-specific hardware registers,
including PPU and APU registers.

C64Foundry does not emulate NES hardware.

The Blargg integration is therefore intended only to provide the
minimum environment necessary for the CPU instruction tests to run.

NES-specific behavior must not be added to the MOS6510 implementation
itself merely to satisfy the Blargg test suite.

Any required compatibility handling belongs exclusively to the
Blargg test harness.

## Combined all_instrs.nes ROM

all_instrs.nes combines the individual instruction tests into a
single ROM.

Unlike the individual 32 KiB test ROMs, the downloaded all_instrs.nes
contains:

16 x 16 KiB = 256 KiB PRG data

and therefore cannot be mapped directly as a single 32 KiB im

