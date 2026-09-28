#include "MOS6510TestUndocumentedAXS.h"

#include <QTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedAXS::MOS6510TestUndocumentedAXS()
{
}

MOS6510TestUndocumentedAXS::~MOS6510TestUndocumentedAXS()
{
}


void MOS6510TestUndocumentedAXS::testAXSImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedX");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    //
    // $CB - AXS/SBX immediate
    //
    //   value  = A & X
    //   result = value - operand
    //
    //   X = result
    //
    // C is set when no borrow occurs.
    // Z and N are determined from the result.
    //
    QTest::newRow("basic")
        << quint8(0xFF)
        << quint8(0x7F)
        << quint8(0x10)
        << quint8(0x6F)
        << true
        << false
        << false;

    QTest::newRow("and-before-subtract")
        << quint8(0x0F)
        << quint8(0xF3)
        << quint8(0x01)
        << quint8(0x02)
        << true
        << false
        << false;

    QTest::newRow("zero")
        << quint8(0xFF)
        << quint8(0x55)
        << quint8(0x55)
        << quint8(0x00)
        << true
        << true
        << false;

    QTest::newRow("borrow")
        << quint8(0x0F)
        << quint8(0x0F)
        << quint8(0x10)
        << quint8(0xFF)
        << false
        << false
        << true;

    QTest::newRow("negative-without-borrow")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x01)
        << quint8(0xFE)
        << true
        << false
        << true;

    QTest::newRow("wrap-around")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x01)
        << quint8(0xFF)
        << false
        << false
        << true;

    QTest::newRow("zero-minus-zero")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x00)
        << true
        << true
        << false;

    QTest::newRow("ff-minus-ff")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x00)
        << true
        << true
        << false;
}


void MOS6510TestUndocumentedAXS::testAXSImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, xRegister);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedX);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(xRegister);

    //
    // Deliberately start with C, Z and N set.
    // AXS must calculate all three flags from
    // the operation result.
    //
    m_cpu.setStatus(
        static_cast<quint8>(MOS6510StatusFlag::Carry) |
        static_cast<quint8>(MOS6510StatusFlag::Zero) |
        static_cast<quint8>(MOS6510StatusFlag::Negative));

    m_memory.writeRAM(0x1000, 0xCB);
    m_memory.writeRAM(0x1001, operand);

    clock();
    clock();

    //
    // AXS modifies X, but not A.
    //
    QCOMPARE(
        m_cpu.accumulator(),
        accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expectedX);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        expectedCarry);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Zero),
        expectedZero);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Negative),
        expectedNegative);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));
}


void MOS6510TestUndocumentedAXS::testAXSImmediateFlags()
{
    //
    // AXS may only modify C, Z and N.
    //
    // I, D and V must remain unchanged.
    //
    const quint8 PreservedFlags =
        static_cast<quint8>(MOS6510StatusFlag::InterruptDisable) |
        static_cast<quint8>(MOS6510StatusFlag::Decimal) |
        static_cast<quint8>(MOS6510StatusFlag::Overflow);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0x80);

    m_cpu.setStatus(
        PreservedFlags);

    m_memory.writeRAM(0x1000, 0xCB);
    m_memory.writeRAM(0x1001, 0x01);

    clock();
    clock();

    //
    // ($FF & $80) - $01 = $7F
    //
    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0xFF));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x7F));

    QVERIFY(
        m_cpu.statusFlag(
            MOS6510StatusFlag::Carry));

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


void MOS6510TestUndocumentedAXS::testAXSImmediateCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0x55);

    m_memory.writeRAM(0x1000, 0xCB);
    m_memory.writeRAM(0x1001, 0x10);

    //
    // Put a known valid opcode behind $CB.
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
        0xCB);

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
        0x10);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    //
    // ($FF & $55) - $10 = $45
    //
    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0xFF));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x45));

    //
    // Cycle 3 must already fetch the next opcode.
    //
    // This proves that $CB consumed exactly two cycles.
    //
    clock();

    verifyRead(
        0x1002,
        0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));
}
