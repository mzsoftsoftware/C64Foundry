#include "MOS6510TestUndocumentedXAA.h"

#include <QTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedXAA::MOS6510TestUndocumentedXAA()
{
}

MOS6510TestUndocumentedXAA::~MOS6510TestUndocumentedXAA()
{
}


void MOS6510TestUndocumentedXAA::testXAAImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expected");

    //
    // $8B - XAA/ANE immediate
    //
    // Deterministic model used by C64Foundry:
    //
    //   result = (A | $EE) & X & operand
    //   A      = result
    //
    // $EE is the same magic constant used by our
    // unstable $AB LAX/ATX immediate model.
    //

    QTest::newRow("all zero")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x00);

    //
    // Makes the $EE magic value directly visible.
    //
    QTest::newRow("magic visible")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0xEE);

    //
    // A contributes the bits missing from $EE:
    //
    // $11 | $EE = $FF
    //
    QTest::newRow("accumulator adds bits")
        << quint8(0x11)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0xFF);

    QTest::newRow("x mask")
        << quint8(0xFF)
        << quint8(0x5A)
        << quint8(0xFF)
        << quint8(0x5A);

    QTest::newRow("operand mask")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x3C)
        << quint8(0x3C);

    //
    // ($01 | $EE) & $F3 & $5F
    //
    // $EF & $F3 = $E3
    // $E3 & $5F = $43
    //
    QTest::newRow("combined mask")
        << quint8(0x01)
        << quint8(0xF3)
        << quint8(0x5F)
        << quint8(0x43);

    //
    // Result bit 7 set.
    //
    QTest::newRow("negative")
        << quint8(0x00)
        << quint8(0x80)
        << quint8(0xFF)
        << quint8(0x80);

    //
    // X and operand masks eliminate each other.
    //
    QTest::newRow("zero result")
        << quint8(0xFF)
        << quint8(0x0F)
        << quint8(0xF0)
        << quint8(0x00);
}


void MOS6510TestUndocumentedXAA::testXAAImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, xRegister);
    QFETCH(quint8, operand);
    QFETCH(quint8, expected);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(xRegister);

    m_memory.writeRAM(0x1000, 0x8B);
    m_memory.writeRAM(0x1001, operand);

    //
    // XAA #imm = 2 cycles.
    //
    clock();
    clock();

    QCOMPARE(
        m_cpu.accumulator(),
        expected);

    //
    // X is only an input and must not be modified.
    //
    QCOMPARE(
        m_cpu.xRegister(),
        xRegister);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    QCOMPARE(
        m_cpu.statusFlag(
            MOS6510StatusFlag::Zero),
        expected == 0x00);

    QCOMPARE(
        m_cpu.statusFlag(
            MOS6510StatusFlag::Negative),
        (expected & 0x80) != 0);
}


void MOS6510TestUndocumentedXAA::testXAAFlags()
{
    //
    // XAA may only modify Z and N.
    //
    // Preserve C, I, D and V.
    //
    const quint8 PreservedFlags =
        static_cast<quint8>(MOS6510StatusFlag::Carry) |
        static_cast<quint8>(MOS6510StatusFlag::InterruptDisable) |
        static_cast<quint8>(MOS6510StatusFlag::Decimal) |
        static_cast<quint8>(MOS6510StatusFlag::Overflow);

    //
    // First case:
    //
    // ($00 | $EE) & $FF & $00 = $00
    //
    // Z must be set.
    // N must be clear.
    //
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0xFF);

    m_cpu.setStatus(
        PreservedFlags |
        static_cast<quint8>(
            MOS6510StatusFlag::Negative));

    m_memory.writeRAM(0x1000, 0x8B);
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    clock();

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x00));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0xFF));

    QVERIFY(
        m_cpu.statusFlag(
            MOS6510StatusFlag::Zero));

    QVERIFY(
        !m_cpu.statusFlag(
            MOS6510StatusFlag::Negative));

    QCOMPARE(
        static_cast<quint8>(
            m_cpu.status() & PreservedFlags),
        PreservedFlags);

    //
    // Second case:
    //
    // ($00 | $EE) & $80 & $FF = $80
    //
    // N must be set.
    // Z must be clear.
    //
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x80);

    m_cpu.setStatus(
        PreservedFlags |
        static_cast<quint8>(
            MOS6510StatusFlag::Zero));

    m_memory.writeRAM(0x1000, 0x8B);
    m_memory.writeRAM(0x1001, 0xFF);

    clock();
    clock();

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x80));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x80));

    QVERIFY(
        m_cpu.statusFlag(
            MOS6510StatusFlag::Negative));

    QVERIFY(
        !m_cpu.statusFlag(
            MOS6510StatusFlag::Zero));

    QCOMPARE(
        static_cast<quint8>(
            m_cpu.status() & PreservedFlags),
        PreservedFlags);

    //
    // Third case:
    //
    // ($FF | $EE) & $7F & $7F = $7F
    //
    // Neither Z nor N may remain set.
    //
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0x7F);

    m_cpu.setStatus(
        PreservedFlags |
        static_cast<quint8>(
            MOS6510StatusFlag::Zero) |
        static_cast<quint8>(
            MOS6510StatusFlag::Negative));

    m_memory.writeRAM(0x1000, 0x8B);
    m_memory.writeRAM(0x1001, 0x7F);

    clock();
    clock();

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x7F));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x7F));

    QVERIFY(
        !m_cpu.statusFlag(
            MOS6510StatusFlag::Zero));

    QVERIFY(
        !m_cpu.statusFlag(
            MOS6510StatusFlag::Negative));

    QCOMPARE(
        static_cast<quint8>(
            m_cpu.status() & PreservedFlags),
        PreservedFlags);
}


void MOS6510TestUndocumentedXAA::testXAAImmediateCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0xFF);

    m_memory.writeRAM(0x1000, 0x8B);
    m_memory.writeRAM(0x1001, 0xFF);

    //
    // Put a known valid opcode behind XAA.
    // $EA is the official NOP.
    //
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Cycle 1:
    //
    // Opcode fetch.
    //
    clock();

    verifyRead(
        0x1000,
        0x8B);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x00));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0xFF));

    //
    // Cycle 2:
    //
    // Read immediate operand and execute XAA.
    //
    clock();

    verifyRead(
        0x1001,
        0xFF);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    //
    // ($00 | $EE) & $FF & $FF = $EE
    //
    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0xEE));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0xFF));

    //
    // Cycle 3 must already fetch the next opcode.
    //
    // This proves that XAA itself consumes exactly
    // two cycles.
    //
    clock();

    verifyRead(
        0x1002,
        0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0xEE));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0xFF));
}
