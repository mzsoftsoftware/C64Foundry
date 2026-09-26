# MOS6510 Performance Reference

This document records reference performance measurements for the
C64Foundry MOS6510 emulator.

The benchmark uses Klaus Dormann's 6502 Functional Test and executes
the complete test workload ten times.

Performance values are informational only and are not used to determine
test success or failure.

## Reference System

CPU: Intel Core i5-12600
OS: EndeavourOS
Architecture: x86_64
Qt: 6.11.2
Compiler: GCC 11.5.0
Build type: Release

## Workload

Test image:

    6502_functional_test.bin

Start address:

    $0400

Success address:

    $3469

Emulated cycles per run:

    96,241,367

Number of runs:

    10

Total emulated cycles:

    962,413,670

## Reference Measurement

Total time:

    15,048.922 ms

Average time per run:

    1,504.892 ms

Minimum time:

    1,495.161 ms

Maximum time:

    1,515.959 ms

Performance:

    63.952 Mcycles/s

PAL C64 realtime factor:

    64.91x

The PAL realtime factor is calculated using a reference CPU frequency
of 0.985248 MHz.

## Notes

Only the emulator clock loop is timed.

CPU initialization, loading the 64 KiB functional-test image, and
result verification are outside the measured interval.

Each benchmark run starts with a freshly initialized CPU and a restored
64 KiB memory image.

The benchmark executes the normal C64Foundry test clock path:

    C64Bus::clock()
    MOS6510::clock()

Results may vary depending on CPU frequency scaling, system load,
compiler version, Qt version, and build configuration.

For meaningful performance comparisons, measurements should be made
on the same system using the same build configuration.
