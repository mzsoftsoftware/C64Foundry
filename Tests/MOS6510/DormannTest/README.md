# MOS6502 Functional Test

Source:
Klaus Dormann  
6502/65C02 functional tests

Upstream repository:
https://github.com/Klaus2m5/6502_65C02_functional_tests

Upstream commit:
7954e2dbb49c469ea286070bf46cdd71aeb29e4b

Files:
- 6502_functional_test.bin
- 6502_functional_test.lst

Binary size:
65536 bytes

Configuration:
- report = 0
- disable_decimal = 0

Load address:
$0000 (complete 64 KiB memory image)

Program start:
$0400

Success trap:
$3469 (JMP $3469)

The original upstream readme and license are included unchanged as
readme.txt and license.txt.


## Optimized MOS6510 Core

The MOS6510 execution hot path was optimized by:

- executing micro-operations directly from the instruction table instead of
  copying them into the CPU state
- integrating instruction fetch/decode/preparation into the CPU clock hot path
- integrating micro-operation dispatch directly into the CPU clock hot path

Benchmark results:

- Emulated cycles/run: 96,241,367
- Runs: 10
- Total emulated cycles: 962,413,670
- Average runtime: 1,168.892 ms/run
- Minimum runtime: 1,160.713 ms
- Maximum runtime: 1,186.191 ms
- Performance: 82.336 Mcycles/s
- PAL C64 equivalent: 83.57x realtime

Compared to the original baseline of 63.952 Mcycles/s, this is an
approximately 28.75% increase in emulation throughput.

