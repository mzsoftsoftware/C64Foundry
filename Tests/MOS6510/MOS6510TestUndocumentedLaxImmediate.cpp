#include "MOS6510TestUndocumentedLaxImmediate.h"

#include <QTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedLaxImmediate::MOS6510TestUndocumentedLaxImmediate()
{
}
MOS6510TestUndocumentedLaxImmediate::~MOS6510TestUndocumentedLaxImmediate()
{
}


void MOS6510TestUndocumentedLaxImmediate::testLaxImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expected");

    //
    // $AB - unstable LAX immediate
    //
    // Model used by C64Foundry:
    //
    //   result = (A | $EE) & operand
    //   A      = result
    //   X      = result
    //
    // $EE models the commonly documented magic constant
    // of the unstable NMOS LAX/ATX immediate opcode.
    //
    QTest::newRow("A00_OperandFF")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0xEE);

    QTest::newRow("A11_OperandFF")
        << quint8(0x11)
        << quint8(0xFF)
        << quint8(0xFF);

    QTest::newRow("A00_Operand55")
        << quint8(0x00)
        << quint8(0x55)
        << quint8(0x44);

    QTest::newRow("A11_Operand55")
        << quint8(0x11)
        << quint8(0x55)
        << quint8(0x55);

    QTest::newRow("AFF_Operand37")
        << quint8(0xFF)
        << quint8(0x37)
        << quint8(0x37);

    QTest::newRow("A00_Operand00")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x00);

    QTest::newRow("A00_Operand80")
        << quint8(0x00)
        << quint8(0x80)
        << quint8(0x80);
}


void MOS6510TestUndocumentedLaxImmediate::testLaxImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expected);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x5A);

    m_memory.writeRAM(0x1000, 0xAB);
    m_memory.writeRAM(0x1001, operand);

    clock();
    clock();

    QCOMPARE(m_cpu.accumulator(), expected);
    QCOMPARE(m_cpu.xRegister(), expected);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}


void MOS6510TestUndocumentedLaxImmediate::testLaxImmediateFlags()
{
    //
    // Preserve C, I, D and V while starting with
    // N and Z set. LAX immediate may only change N/Z.
    //
    const quint8 PreservedFlags =
        static_cast<quint8>(MOS6510StatusFlag::Carry) |
        static_cast<quint8>(MOS6510StatusFlag::InterruptDisable) |
        static_cast<quint8>(MOS6510StatusFlag::Decimal) |
        static_cast<quint8>(MOS6510StatusFlag::Overflow);

    //
    // result = ($00 | $EE) & $00 = $00
    //
    // Z must be set, N must be clear.
    //
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x5A);

    m_cpu.setStatus(
        PreservedFlags |
        static_cast<quint8>(MOS6510StatusFlag::Negative));

    m_memory.writeRAM(0x1000, 0xAB);
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    clock();

    QCOMPARE(m_cpu.accumulator(), quint8(0x00));
    QCOMPARE(m_cpu.xRegister(), quint8(0x00));

    QVERIFY(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Zero));
    QVERIFY(!(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Negative)));

    QCOMPARE(
        static_cast<quint8>(
            m_cpu.status() & PreservedFlags),
        PreservedFlags);

    //
    // result = ($00 | $EE) & $80 = $80
    //
    // N must be set, Z must be clear.
    //
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x5A);

    m_cpu.setStatus(
            PreservedFlags |
            static_cast<quint8>(MOS6510StatusFlag::Zero));

    m_memory.writeRAM(0x1000, 0xAB);
    m_memory.writeRAM(0x1001, 0x80);

    clock();
    clock();

    QCOMPARE(m_cpu.accumulator(), quint8(0x80));
    QCOMPARE(m_cpu.xRegister(), quint8(0x80));

    QVERIFY(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Negative));
    QVERIFY(!(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Zero)));

    QCOMPARE(
            m_cpu.status() & PreservedFlags,
        PreservedFlags);

    //
    // result = ($FF | $EE) & $7F = $7F
    //
    // Neither N nor Z may be set.
    //
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0x5A);

    m_cpu.setStatus(
            PreservedFlags |
            static_cast<quint8>(MOS6510StatusFlag::Negative) |
            static_cast<quint8>(MOS6510StatusFlag::Zero));

    m_memory.writeRAM(0x1000, 0xAB);
    m_memory.writeRAM(0x1001, 0x7F);

    clock();
    clock();

    QCOMPARE(m_cpu.accumulator(), quint8(0x7F));
    QCOMPARE(m_cpu.xRegister(), quint8(0x7F));

    QVERIFY(!(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Negative)));
    QVERIFY(!(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Zero)));

    QCOMPARE(
        static_cast<quint8>(
            m_cpu.status() & PreservedFlags),
        PreservedFlags);
}


void MOS6510TestUndocumentedLaxImmediate::testLaxImmediateCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);
    m_cpu.setXRegister(0x5A);

    m_memory.writeRAM(0x1000, 0xAB);
    m_memory.writeRAM(0x1001, 0x55);

    //
    // Put a known valid opcode behind $AB.
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
        0xAB);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    //
    // Cycle 2:
    //
    // Immediate operand fetch.
    //
    clock();

    verifyRead(
        0x1001,
        0x55);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x44));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x44));

    //
    // Cycle 3 must already fetch the next opcode.
    //
    // This proves that $AB consumed exactly two cycles.
    //
    clock();

    verifyRead(
        0x1002,
        0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));
}
