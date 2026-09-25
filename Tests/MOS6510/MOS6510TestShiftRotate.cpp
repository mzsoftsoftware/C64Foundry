#include "MOS6510TestShiftRotate.h"

#include <QTest>


MOS6510TestShiftRotate::MOS6510TestShiftRotate()
{
}

MOS6510TestShiftRotate::~MOS6510TestShiftRotate()
{
}

void MOS6510TestShiftRotate::init()
{
    setupCpu();
    m_cpu.setProgramCounter(0x1000);
}


// ============================================================================
// ASL
// ============================================================================

void MOS6510TestShiftRotate::testAslAccumulator_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 -> 00") << quint8(0x00) << quint8(0x00) << false << true  << false;
    QTest::newRow("01 -> 02") << quint8(0x01) << quint8(0x02) << false << false << false;
    QTest::newRow("40 -> 80") << quint8(0x40) << quint8(0x80) << false << false << true;
    QTest::newRow("7F -> FE") << quint8(0x7F) << quint8(0xFE) << false << false << true;
    QTest::newRow("80 -> 00") << quint8(0x80) << quint8(0x00) << true  << true  << false;
    QTest::newRow("81 -> 02") << quint8(0x81) << quint8(0x02) << true  << false << false;
    QTest::newRow("FF -> FE") << quint8(0xFF) << quint8(0xFE) << true  << false << true;
}

void MOS6510TestShiftRotate::testAslAccumulator()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x0A);
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setAccumulator(value);
    m_cpu.setStatus(0x7C);
    m_cpu.setXRegister(0x23);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    m_cpu.clock();      // C1 opcode
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();      // C2 ASL A
    QCOMPARE(m_cpu.accumulator(), expectedValue);

    quint8 expectedStatus = 0x7C;

    if (expectedCarry)
        expectedStatus |= 0x01;

    if (expectedZero)
        expectedStatus |= 0x02;

    if (expectedNegative)
        expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);
    QCOMPARE(m_cpu.xRegister(), quint8(0x23));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    m_cpu.clock();      // C3 next opcode
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}

void MOS6510TestShiftRotate::testAslZeroPage_data()
{
    testAslAccumulator_data();
}

void MOS6510TestShiftRotate::testAslZeroPage()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x06);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0020, value);

    m_cpu.setStatus(0x7C);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2
    m_cpu.clock();      // C3 read
    QCOMPARE(m_memory.readRAM(0x0020), value);

    m_cpu.clock();      // C4 old write
    QCOMPARE(m_memory.readRAM(0x0020), value);

    m_cpu.clock();      // C5 new write
    QCOMPARE(m_memory.readRAM(0x0020), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();      // C6
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testAslZeroPageX_data()
{
    testAslAccumulator_data();
}

void MOS6510TestShiftRotate::testAslZeroPageX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x16);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0025, value);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7C);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2
    m_cpu.clock();      // C3 indexed dummy
    m_cpu.clock();      // C4 read
    QCOMPARE(m_memory.readRAM(0x0025), value);

    m_cpu.clock();      // C5 old write
    QCOMPARE(m_memory.readRAM(0x0025), value);

    m_cpu.clock();      // C6 new write
    QCOMPARE(m_memory.readRAM(0x0025), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();      // C7
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testAslZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x16);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0003, 0x41);
    m_memory.writeRAM(0x00FE, 0x11);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7C);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x82));
    QCOMPARE(m_memory.readRAM(0x00FE), quint8(0x11));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testAslAbsolute_data()
{
    testAslAccumulator_data();
}

void MOS6510TestShiftRotate::testAslAbsolute()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x0E);
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    m_cpu.setStatus(0x7C);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2
    m_cpu.clock();      // C3
    m_cpu.clock();      // C4 read
    m_cpu.clock();      // C5 old write
    m_cpu.clock();      // C6 new write

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testAslAbsoluteX_data()
{
    testAslAccumulator_data();
}

void MOS6510TestShiftRotate::testAslAbsoluteX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x1E);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7C);

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testAslAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x1E);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2403, 0x41);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7C);

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x82));
    QCOMPARE(m_cpu.status(), quint8(0xFC));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}


// ============================================================================
// LSR
// ============================================================================

void MOS6510TestShiftRotate::testLsrAccumulator_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedZero");

    QTest::newRow("00 -> 00") << quint8(0x00) << quint8(0x00) << false << true;
    QTest::newRow("01 -> 00") << quint8(0x01) << quint8(0x00) << true  << true;
    QTest::newRow("02 -> 01") << quint8(0x02) << quint8(0x01) << false << false;
    QTest::newRow("03 -> 01") << quint8(0x03) << quint8(0x01) << true  << false;
    QTest::newRow("80 -> 40") << quint8(0x80) << quint8(0x40) << false << false;
    QTest::newRow("FF -> 7F") << quint8(0xFF) << quint8(0x7F) << true  << false;
}

void MOS6510TestShiftRotate::testLsrAccumulator()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);

    m_memory.writeRAM(0x1000, 0x4A);
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setAccumulator(value);

    // N deliberately set. LSR must always clear it.
    m_cpu.setStatus(0xFC);

    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.accumulator(), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry) expectedStatus |= 0x01;
    if (expectedZero)  expectedStatus |= 0x02;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}

void MOS6510TestShiftRotate::testLsrZeroPage_data()
{
    testLsrAccumulator_data();
}

void MOS6510TestShiftRotate::testLsrZeroPage()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);

    m_memory.writeRAM(0x1000, 0x46);    // LSR $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0020, value);

    m_cpu.setStatus(0xFC);

    m_cpu.clock();      // C1 opcode
    m_cpu.clock();      // C2 address

    m_cpu.clock();      // C3 read
    QCOMPARE(m_memory.readRAM(0x0020), value);

    m_cpu.clock();      // C4 old write
    QCOMPARE(m_memory.readRAM(0x0020), value);

    m_cpu.clock();      // C5 new write
    QCOMPARE(m_memory.readRAM(0x0020), expectedValue);

    quint8 expectedStatus = 0x7C;

    if (expectedCarry)
        expectedStatus |= 0x01;

    if (expectedZero)
        expectedStatus |= 0x02;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();      // C6 next opcode
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testLsrZeroPageX_data()
{
    testLsrAccumulator_data();
}

void MOS6510TestShiftRotate::testLsrZeroPageX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);

    m_memory.writeRAM(0x1000, 0x56);    // LSR $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0025, value);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0xFC);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2
    m_cpu.clock();      // C3 indexed dummy

    m_cpu.clock();      // C4 read
    QCOMPARE(m_memory.readRAM(0x0025), value);

    m_cpu.clock();      // C5 old write
    QCOMPARE(m_memory.readRAM(0x0025), value);

    m_cpu.clock();      // C6 new write
    QCOMPARE(m_memory.readRAM(0x0025), expectedValue);

    quint8 expectedStatus = 0x7C;

    if (expectedCarry)
        expectedStatus |= 0x01;

    if (expectedZero)
        expectedStatus |= 0x02;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();      // C7
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testLsrZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x56);    // LSR $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0003, 0x82);
    m_memory.writeRAM(0x00FE, 0x11);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0xFC);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x41));
    QCOMPARE(m_memory.readRAM(0x00FE), quint8(0x11));
    QCOMPARE(m_cpu.status(), quint8(0x7C));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testLsrAbsolute_data()
{
    testLsrAccumulator_data();
}

void MOS6510TestShiftRotate::testLsrAbsolute()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);

    m_memory.writeRAM(0x1000, 0x4E);    // LSR $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    m_cpu.setStatus(0xFC);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;

    if (expectedCarry)
        expectedStatus |= 0x01;

    if (expectedZero)
        expectedStatus |= 0x02;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testLsrAbsoluteX_data()
{
    testLsrAccumulator_data();
}

void MOS6510TestShiftRotate::testLsrAbsoluteX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);

    m_memory.writeRAM(0x1000, 0x5E);    // LSR $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0xFC);

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;

    if (expectedCarry)
        expectedStatus |= 0x01;

    if (expectedZero)
        expectedStatus |= 0x02;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testLsrAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x5E);    // LSR $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2403, 0x82);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0xFC);

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x41));
    QCOMPARE(m_cpu.status(), quint8(0x7C));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}
void MOS6510TestShiftRotate::testRolAccumulator_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<bool>("carryIn");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 C0 -> 00") << quint8(0x00) << false << quint8(0x00) << false << true  << false;
    QTest::newRow("00 C1 -> 01") << quint8(0x00) << true  << quint8(0x01) << false << false << false;

    QTest::newRow("40 C0 -> 80") << quint8(0x40) << false << quint8(0x80) << false << false << true;
    QTest::newRow("40 C1 -> 81") << quint8(0x40) << true  << quint8(0x81) << false << false << true;

    QTest::newRow("80 C0 -> 00") << quint8(0x80) << false << quint8(0x00) << true  << true  << false;
    QTest::newRow("80 C1 -> 01") << quint8(0x80) << true  << quint8(0x01) << true  << false << false;

    QTest::newRow("FF C0 -> FE") << quint8(0xFF) << false << quint8(0xFE) << true  << false << true;
    QTest::newRow("FF C1 -> FF") << quint8(0xFF) << true  << quint8(0xFF) << true  << false << true;
}

void MOS6510TestShiftRotate::testRolAccumulator()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x2A);    // ROL A
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setAccumulator(value);

    quint8 initialStatus = 0x7C;

    if (carryIn)
        initialStatus |= 0x01;

    m_cpu.setStatus(initialStatus);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2

    QCOMPARE(m_cpu.accumulator(), expectedValue);

    quint8 expectedStatus = 0x7C;

    if (expectedCarry)
        expectedStatus |= 0x01;
    if (expectedZero)
        expectedStatus |= 0x02;
    if (expectedNegative)
        expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}

void MOS6510TestShiftRotate::testRolZeroPage_data()
{
    testRolAccumulator_data();
}

void MOS6510TestShiftRotate::testRolZeroPage()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x26);    // ROL $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0020, value);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;

    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 5; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0020), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testRolZeroPageX_data()
{
    testRolAccumulator_data();
}

void MOS6510TestShiftRotate::testRolZeroPageX()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x36);    // ROL $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0025, value);

    m_cpu.setXRegister(0x05);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;

    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0025), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testRolZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x36);    // ROL $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0003, 0x40);
    m_memory.writeRAM(0x00FE, 0x11);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);              // Carry set

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x81));
    QCOMPARE(m_memory.readRAM(0x00FE), quint8(0x11));
    QCOMPARE(m_cpu.status(), quint8(0xFC));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testRolAbsolute_data()
{
    testRolAccumulator_data();
}

void MOS6510TestShiftRotate::testRolAbsolute()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x2E);    // ROL $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;

    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testRolAbsoluteX_data()
{
    testRolAccumulator_data();
}

void MOS6510TestShiftRotate::testRolAbsoluteX()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x3E);    // ROL $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    m_cpu.setXRegister(0x05);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;

    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testRolAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x3E);    // ROL $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2403, 0x40);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);              // Carry set

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x81));
    QCOMPARE(m_cpu.status(), quint8(0xFC));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testRorAccumulator_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<bool>("carryIn");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 C0 -> 00") << quint8(0x00) << false << quint8(0x00) << false << true  << false;
    QTest::newRow("00 C1 -> 80") << quint8(0x00) << true  << quint8(0x80) << false << false << true;

    QTest::newRow("01 C0 -> 00") << quint8(0x01) << false << quint8(0x00) << true  << true  << false;
    QTest::newRow("01 C1 -> 80") << quint8(0x01) << true  << quint8(0x80) << true  << false << true;

    QTest::newRow("02 C0 -> 01") << quint8(0x02) << false << quint8(0x01) << false << false << false;
    QTest::newRow("02 C1 -> 81") << quint8(0x02) << true  << quint8(0x81) << false << false << true;

    QTest::newRow("FF C0 -> 7F") << quint8(0xFF) << false << quint8(0x7F) << true  << false << false;
    QTest::newRow("FF C1 -> FF") << quint8(0xFF) << true  << quint8(0xFF) << true  << false << true;
}

void MOS6510TestShiftRotate::testRorAccumulator()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x6A);    // ROR A
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setAccumulator(value);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;

    m_cpu.setStatus(initialStatus);

    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.accumulator(), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}

void MOS6510TestShiftRotate::testRorZeroPage_data()
{
    testRorAccumulator_data();
}

void MOS6510TestShiftRotate::testRorZeroPage()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x66);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0020, value);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;
    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 5; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0020), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testRorZeroPageX_data()
{
    testRorAccumulator_data();
}

void MOS6510TestShiftRotate::testRorZeroPageX()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x76);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0025, value);

    m_cpu.setXRegister(0x05);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;
    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0025), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testRorZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x76);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0003, 0x02);
    m_memory.writeRAM(0x00FE, 0x11);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);              // Carry set

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x81));
    QCOMPARE(m_memory.readRAM(0x00FE), quint8(0x11));
    QCOMPARE(m_cpu.status(), quint8(0xFC));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestShiftRotate::testRorAbsolute_data()
{
    testRorAccumulator_data();
}

void MOS6510TestShiftRotate::testRorAbsolute()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x6E);
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;
    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 6; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testRorAbsoluteX_data()
{
    testRorAccumulator_data();
}

void MOS6510TestShiftRotate::testRorAbsoluteX()
{
    QFETCH(quint8, value);
    QFETCH(bool, carryIn);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0x7E);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2345, value);

    m_cpu.setXRegister(0x05);

    quint8 initialStatus = 0x7C;
    if (carryIn)
        initialStatus |= 0x01;
    m_cpu.setStatus(initialStatus);

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7C;
    if (expectedCarry)    expectedStatus |= 0x01;
    if (expectedZero)     expectedStatus |= 0x02;
    if (expectedNegative) expectedStatus |= 0x80;

    QCOMPARE(m_cpu.status(), expectedStatus);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestShiftRotate::testRorAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x7E);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x2403, 0x02);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);              // Carry set

    for (int i = 0; i < 7; ++i)
        m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x81));
    QCOMPARE(m_cpu.status(), quint8(0xFC));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

