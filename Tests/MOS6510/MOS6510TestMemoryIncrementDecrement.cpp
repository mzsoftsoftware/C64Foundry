#include "MOS6510TestMemoryIncrementDecrement.h"

#include <QTest>


MOS6510TestMemoryIncrementDecrement::MOS6510TestMemoryIncrementDecrement()
{
}
MOS6510TestMemoryIncrementDecrement::~MOS6510TestMemoryIncrementDecrement()
{
}

void MOS6510TestMemoryIncrementDecrement::init()
{
    setupCpu();
    m_cpu.setProgramCounter(0x1000);
}

void MOS6510TestMemoryIncrementDecrement::testIncrementZeroPage_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 -> 01")
        << quint8(0x00)
        << quint8(0x01)
        << false
        << false;

    QTest::newRow("01 -> 02")
        << quint8(0x01)
        << quint8(0x02)
        << false
        << false;

    QTest::newRow("7E -> 7F")
        << quint8(0x7E)
        << quint8(0x7F)
        << false
        << false;

    QTest::newRow("7F -> 80")
        << quint8(0x7F)
        << quint8(0x80)
        << false
        << true;

    QTest::newRow("80 -> 81")
        << quint8(0x80)
        << quint8(0x81)
        << false
        << true;

    QTest::newRow("FE -> FF")
        << quint8(0xFE)
        << quint8(0xFF)
        << false
        << true;

    QTest::newRow("FF -> 00")
        << quint8(0xFF)
        << quint8(0x00)
        << true
        << false;
}


void MOS6510TestMemoryIncrementDecrement::testIncrementZeroPage()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xE6);    // INC $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, value);

    // Deliberately set all flags except Zero and Negative.
    // INC must preserve C, I, D, B, U and V.
    m_cpu.setStatus(0x7D);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x23);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    // Cycle 1: fetch opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_memory.readRAM(0x0020), value);

    // Cycle 2: read zero-page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0020), value);

    // Cycle 3: read old value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0020), value);

    // Cycle 4: dummy write of old value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0020), value);

    // Cycle 5: write incremented value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0020), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    // INC must not modify registers or SP.
    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x23));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    // Cycle 6: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestMemoryIncrementDecrement::testIncrementZeroPageX_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 -> 01")
        << quint8(0x00)
        << quint8(0x01)
        << false
        << false;

    QTest::newRow("01 -> 02")
        << quint8(0x01)
        << quint8(0x02)
        << false
        << false;

    QTest::newRow("7E -> 7F")
        << quint8(0x7E)
        << quint8(0x7F)
        << false
        << false;

    QTest::newRow("7F -> 80")
        << quint8(0x7F)
        << quint8(0x80)
        << false
        << true;

    QTest::newRow("80 -> 81")
        << quint8(0x80)
        << quint8(0x81)
        << false
        << true;

    QTest::newRow("FE -> FF")
        << quint8(0xFE)
        << quint8(0xFF)
        << false
        << true;

    QTest::newRow("FF -> 00")
        << quint8(0xFF)
        << quint8(0x00)
        << true
        << false;
}
void MOS6510TestMemoryIncrementDecrement::testIncrementZeroPageX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xF6);    // INC $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0025, value);

    m_cpu.setStatus(0x7D);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    // Cycle 1: fetch opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // Cycle 2: read zero-page base address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // Cycle 3: indexed zero-page dummy read
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // Cycle 4: read old value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // Cycle 5: dummy write of old value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // Cycle 6: write incremented value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x0025), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    // Cycle 7: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}
void MOS6510TestMemoryIncrementDecrement::testIncrementZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xF6);    // INC $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0003, 0x41);

    // Sentinel: must not be modified.
    m_memory.writeRAM(0x00FE, 0x11);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);

    // Cycle 1: fetch opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: read zero-page base address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: dummy read and apply X with zero-page wrap
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 4: read old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x41));

    // Cycle 5: dummy write old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x41));

    // Cycle 6: write new value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x42));

    // Unindexed address must not have been modified.
    QCOMPARE(m_memory.readRAM(0x00FE), quint8(0x11));

    QCOMPARE(m_cpu.status(), quint8(0x7D));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));

    // Cycle 7: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestMemoryIncrementDecrement::testIncrementAbsolute_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 -> 01")
        << quint8(0x00)
        << quint8(0x01)
        << false
        << false;

    QTest::newRow("01 -> 02")
        << quint8(0x01)
        << quint8(0x02)
        << false
        << false;

    QTest::newRow("7E -> 7F")
        << quint8(0x7E)
        << quint8(0x7F)
        << false
        << false;

    QTest::newRow("7F -> 80")
        << quint8(0x7F)
        << quint8(0x80)
        << false
        << true;

    QTest::newRow("80 -> 81")
        << quint8(0x80)
        << quint8(0x81)
        << false
        << true;

    QTest::newRow("FE -> FF")
        << quint8(0xFE)
        << quint8(0xFF)
        << false
        << true;

    QTest::newRow("FF -> 00")
        << quint8(0xFF)
        << quint8(0x00)
        << true
        << false;
}
void MOS6510TestMemoryIncrementDecrement::testIncrementAbsolute()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xEE);    // INC $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x2345, value);

    m_cpu.setStatus(0x7D);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x23);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    // Cycle 1: fetch opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 2: read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 3: read address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 4: read old value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 5: dummy write of old value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 6: write incremented value
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x23));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    // Cycle 7: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestMemoryIncrementDecrement::testIncrementAbsoluteX_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("00 -> 01")
        << quint8(0x00)
        << quint8(0x01)
        << false
        << false;

    QTest::newRow("01 -> 02")
        << quint8(0x01)
        << quint8(0x02)
        << false
        << false;

    QTest::newRow("7E -> 7F")
        << quint8(0x7E)
        << quint8(0x7F)
        << false
        << false;

    QTest::newRow("7F -> 80")
        << quint8(0x7F)
        << quint8(0x80)
        << false
        << true;

    QTest::newRow("80 -> 81")
        << quint8(0x80)
        << quint8(0x81)
        << false
        << true;

    QTest::newRow("FE -> FF")
        << quint8(0xFE)
        << quint8(0xFF)
        << false
        << true;

    QTest::newRow("FF -> 00")
        << quint8(0xFF)
        << quint8(0x00)
        << true
        << false;
}
void MOS6510TestMemoryIncrementDecrement::testIncrementAbsoluteX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xFE);    // INC $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x2345, value);

    m_cpu.setStatus(0x7D);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    // Cycle 1: fetch opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 2: read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 3: read address high and add X
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 4: indexed dummy read
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 5: read old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 6: dummy write old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2345), value);

    // Cycle 7: write incremented value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    // Cycle 8: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}
void MOS6510TestMemoryIncrementDecrement::testIncrementAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xFE);    // INC $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x2403, 0x41);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);

    // Cycle 1: fetch opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: read address high and add X
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: indexed dummy read
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x41));

    // Cycle 5: read old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x41));

    // Cycle 6: dummy write old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x41));

    // Cycle 7: write incremented value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x42));

    QCOMPARE(m_cpu.status(), quint8(0x7D));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));

    // Cycle 8: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestMemoryIncrementDecrement::testDecrementZeroPage_data()
{
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("01 -> 00")
        << quint8(0x01) << quint8(0x00) << true  << false;

    QTest::newRow("02 -> 01")
        << quint8(0x02) << quint8(0x01) << false << false;

    QTest::newRow("7F -> 7E")
        << quint8(0x7F) << quint8(0x7E) << false << false;

    QTest::newRow("80 -> 7F")
        << quint8(0x80) << quint8(0x7F) << false << false;

    QTest::newRow("81 -> 80")
        << quint8(0x81) << quint8(0x80) << false << true;

    QTest::newRow("FF -> FE")
        << quint8(0xFF) << quint8(0xFE) << false << true;

    QTest::newRow("00 -> FF")
        << quint8(0x00) << quint8(0xFF) << false << true;
}
void MOS6510TestMemoryIncrementDecrement::testDecrementZeroPageX_data()
{
    testDecrementZeroPage_data();
}

void MOS6510TestMemoryIncrementDecrement::testDecrementAbsolute_data()
{
    testDecrementZeroPage_data();
}

void MOS6510TestMemoryIncrementDecrement::testDecrementAbsoluteX_data()
{
    testDecrementZeroPage_data();
}

void MOS6510TestMemoryIncrementDecrement::testDecrementZeroPage()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xC6);    // DEC $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, value);

    m_cpu.setStatus(0x7D);
    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x23);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    // C1: opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // C2: zero-page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // C3: read old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0020), value);

    // C4: write old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0020), value);

    // C5: write decremented value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0020), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x23));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    // C6: next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}
void MOS6510TestMemoryIncrementDecrement::testDecrementZeroPageX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xD6);    // DEC $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0025, value);

    m_cpu.setStatus(0x7D);
    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    // C1: opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // C2: base address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // C3: indexed dummy read
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // C4: read old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // C5: write old value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0025), value);

    // C6: write decremented value
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x0025), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    // C7: next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}
void MOS6510TestMemoryIncrementDecrement::testDecrementZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xD6);    // DEC $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x00FE, 0x11);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2
    m_cpu.clock();      // C3
    m_cpu.clock();      // C4

    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x42));

    m_cpu.clock();      // C5
    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x42));

    m_cpu.clock();      // C6
    QCOMPARE(m_memory.readRAM(0x0003), quint8(0x41));

    QCOMPARE(m_memory.readRAM(0x00FE), quint8(0x11));
    QCOMPARE(m_cpu.status(), quint8(0x7D));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));

    m_cpu.clock();      // C7
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}
void MOS6510TestMemoryIncrementDecrement::testDecrementAbsolute()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xCE);    // DEC $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x2345, value);

    m_cpu.setStatus(0x7D);
    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x23);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    m_cpu.clock();      // C1
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();      // C2
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    m_cpu.clock();      // C3
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    m_cpu.clock();      // C4
    QCOMPARE(m_memory.readRAM(0x2345), value);

    m_cpu.clock();      // C5
    QCOMPARE(m_memory.readRAM(0x2345), value);

    m_cpu.clock();      // C6
    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x23));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    m_cpu.clock();      // C7
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}
void MOS6510TestMemoryIncrementDecrement::testDecrementAbsoluteX()
{
    QFETCH(quint8, value);
    QFETCH(quint8, expectedValue);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    m_memory.writeRAM(0x1000, 0xDE);    // DEC $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x2345, value);

    m_cpu.setStatus(0x7D);
    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x45);
    m_cpu.setStackPointer(0xFD);

    m_cpu.clock();      // C1
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();      // C2
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    m_cpu.clock();      // C3
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    m_cpu.clock();      // C4 indexed dummy read
    QCOMPARE(m_memory.readRAM(0x2345), value);

    m_cpu.clock();      // C5 read old
    QCOMPARE(m_memory.readRAM(0x2345), value);

    m_cpu.clock();      // C6 write old
    QCOMPARE(m_memory.readRAM(0x2345), value);

    m_cpu.clock();      // C7 write new
    QCOMPARE(m_memory.readRAM(0x2345), expectedValue);

    quint8 expectedStatus = 0x7D;

    if (expectedZero)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);

    if (expectedNegative)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    QCOMPARE(m_cpu.status(), expectedStatus);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));
    QCOMPARE(m_cpu.yRegister(), quint8(0x45));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    m_cpu.clock();      // C8
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}
void MOS6510TestMemoryIncrementDecrement::testDecrementAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xDE);    // DEC $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x2403, 0x42);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x7D);

    m_cpu.clock();      // C1
    m_cpu.clock();      // C2
    m_cpu.clock();      // C3

    m_cpu.clock();      // C4 indexed wrong-page dummy read
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x42));

    m_cpu.clock();      // C5 read old
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x42));

    m_cpu.clock();      // C6 write old
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x42));

    m_cpu.clock();      // C7 write new
    QCOMPARE(m_memory.readRAM(0x2403), quint8(0x41));

    QCOMPARE(m_cpu.status(), quint8(0x7D));
    QCOMPARE(m_cpu.xRegister(), quint8(0x05));

    m_cpu.clock();      // C8
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}
