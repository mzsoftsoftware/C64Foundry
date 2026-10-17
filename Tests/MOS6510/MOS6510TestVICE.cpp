#include "MOS6510TestVICE.h"

#include <QTest>


MOS6510TestVICE::MOS6510TestVICE()
{
}

MOS6510TestVICE::~MOS6510TestVICE()
{
}

// --------------------------------------------------------------------------------------------
// VICE: CPU/decimalmode/adc00.prg
//
// Suspected first failing case:
//   A = $00
//   ADC #$40
//   Carry = 0
//   Decimal mode enabled
//
// Expected NMOS 6510 result:
//   A = $40
//   N = 0, V = 0, Z = 0, C = 0, D = 1
//
// This is a regression test for the observed VICE failure.
// --------------------------------------------------------------------------------------------

void MOS6510TestVICE::testAdc00FirstFailure()
{
    setupCpu();

    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x00);
    m_cpu.setYRegister(0x00);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x3C);

    //
    // Execute the original instruction sequence.
    //
    m_memory.writeRAM(0x1000, 0xF8); // SED
    m_memory.writeRAM(0x1001, 0x18); // CLC
    m_memory.writeRAM(0x1002, 0x69); // ADC #$40
    m_memory.writeRAM(0x1003, 0x40);
    m_memory.writeRAM(0x1004, 0xEA); // NOP

    m_cpu.setProgramCounter(0x1000);

    //
    // SED: 2 cycles
    //
    clock();
    clock();

    QCOMPARE(m_cpu.status() & quint8(0x08), quint8(0x08));

    //
    // CLC: 2 cycles
    //
    clock();
    clock();

    QCOMPARE(m_cpu.status() & quint8(0x01), quint8(0x00));

    //
    // ADC immediate: 2 cycles
    //
    clock();
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), quint8(0x40));

    //
    // Compare relevant status flags:
    // N V D Z C
    //
    QCOMPARE(m_cpu.status() & quint8(0xCB), quint8(0x08));
}

// --------------------------------------------------------------------------------------------
// VICE: CPU/decimalmode/adc00.prg
//
// Reproduces the instruction sequence used by VICE to calculate and
// store the accumulator and status flags.
//
// A = $00
// Operand = $40
// Carry = 0
// Decimal mode enabled
//
// Expected:
//   $0500 = $40 (accumulator)
//   $0600 = $08 (masked status flags)
// --------------------------------------------------------------------------------------------

void MOS6510TestVICE::testAdc00InstructionSequence()
{
    setupCpu();

    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x00);
    m_cpu.setYRegister(0x00);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x34);

    m_memory.writeRAM(0x1000, 0xF8); // SED
    m_memory.writeRAM(0x1001, 0xA2); // LDX #$00
    m_memory.writeRAM(0x1002, 0x00);
    m_memory.writeRAM(0x1003, 0x8A); // TXA
    m_memory.writeRAM(0x1004, 0x18); // CLC
    m_memory.writeRAM(0x1005, 0x69); // ADC #$40
    m_memory.writeRAM(0x1006, 0x40);
    m_memory.writeRAM(0x1007, 0x9D); // STA $0500,X
    m_memory.writeRAM(0x1008, 0x00);
    m_memory.writeRAM(0x1009, 0x05);
    m_memory.writeRAM(0x100A, 0x08); // PHP
    m_memory.writeRAM(0x100B, 0x68); // PLA
    m_memory.writeRAM(0x100C, 0x29); // AND #$CB
    m_memory.writeRAM(0x100D, 0xCB);
    m_memory.writeRAM(0x100E, 0x9D); // STA $0600,X
    m_memory.writeRAM(0x100F, 0x00);
    m_memory.writeRAM(0x1010, 0x06);
    m_memory.writeRAM(0x1011, 0xEA); // NOP

    m_cpu.setProgramCounter(0x1000);

    // SED: 2 cycles
    clock();
    clock();

    // LDX immediate: 2 cycles
    clock();
    clock();

    // TXA: 2 cycles
    clock();
    clock();

    // CLC: 2 cycles
    clock();
    clock();

    // ADC immediate: 2 cycles
    clock();
    clock();

    QCOMPARE(m_cpu.accumulator(), quint8(0x40));
    QCOMPARE(m_cpu.status() & quint8(0xCB), quint8(0x08));

    // STA absolute,X: 5 cycles
    for (int cycle = 0; cycle < 5; ++cycle)
        clock();

    QCOMPARE(m_memory.readRAM(0x0500), quint8(0x40));

    // PHP: 3 cycles
    for (int cycle = 0; cycle < 3; ++cycle)
        clock();

    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_memory.readRAM(0x01FF) & quint8(0xCB), quint8(0x08));

    // PLA: 4 cycles
    for (int cycle = 0; cycle < 4; ++cycle)
        clock();

    QCOMPARE(m_cpu.accumulator() & quint8(0xCB), quint8(0x08));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // AND immediate: 2 cycles
    clock();
    clock();

    QCOMPARE(m_cpu.accumulator(), quint8(0x08));

    // STA absolute,X: 5 cycles
    for (int cycle = 0; cycle < 5; ++cycle)
        clock();

    QCOMPARE(m_memory.readRAM(0x0600), quint8(0x08));
    QCOMPARE(m_cpu.programCounter(), quint16(0x1011));
}

// --------------------------------------------------------------------------------------------
// VICE adc00: Self-modifying ADC immediate operand
//
// The CPU executes ADC #$10, modifies its operand through STA,
// and executes the same ADC instruction again.
//
// First execution:  $00 + $10 = $10
// Second execution: $00 + $40 = $40
// --------------------------------------------------------------------------------------------

void MOS6510TestVICE::testAdc00SelfModifyingCode()
{
    setupCpu();

    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x00);
    m_cpu.setYRegister(0x00);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x34);

    m_memory.writeRAM(0x1000, 0xF8); // SED
    m_memory.writeRAM(0x1001, 0x18); // CLC
    m_memory.writeRAM(0x1002, 0x69); // ADC #$10
    m_memory.writeRAM(0x1003, 0x10);
    m_memory.writeRAM(0x1004, 0xA9); // LDA #$40
    m_memory.writeRAM(0x1005, 0x40);
    m_memory.writeRAM(0x1006, 0x8D); // STA $1003
    m_memory.writeRAM(0x1007, 0x03);
    m_memory.writeRAM(0x1008, 0x10);
    m_memory.writeRAM(0x1009, 0xA9); // LDA #$00
    m_memory.writeRAM(0x100A, 0x00);
    m_memory.writeRAM(0x100B, 0x18); // CLC
    m_memory.writeRAM(0x100C, 0x4C); // JMP $1002
    m_memory.writeRAM(0x100D, 0x02);
    m_memory.writeRAM(0x100E, 0x10);
    m_memory.writeRAM(0x100F, 0xEA); // NOP

    m_cpu.setProgramCounter(0x1000);

    // SED: 2 cycles
    clock();
    clock();

    // CLC: 2 cycles
    clock();
    clock();

    // First ADC #$10: 2 cycles
    clock();
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), quint8(0x10));
    QCOMPARE(m_memory.readRAM(0x1003), quint8(0x10));

    // LDA #$40: 2 cycles
    clock();
    clock();

    // STA $1003: 4 cycles
    for (int cycle = 0; cycle < 4; ++cycle)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1009));
    QCOMPARE(m_memory.readRAM(0x1003), quint8(0x40));

    // LDA #$00: 2 cycles
    clock();
    clock();

    // CLC: 2 cycles
    clock();
    clock();

    // JMP $1002: 3 cycles
    for (int cycle = 0; cycle < 3; ++cycle)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), quint8(0x00));

    // Execute the modified ADC #$40: 2 cycles
    clock();
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), quint8(0x40));
    QCOMPARE(m_cpu.status() & quint8(0xCB), quint8(0x08));
}
