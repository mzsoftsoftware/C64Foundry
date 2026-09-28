#include "MOS6510TestUndocumentedSBC.h"

#include <QtTest>


MOS6510TestUndocumentedSBC::MOS6510TestUndocumentedSBC()
{
}

MOS6510TestUndocumentedSBC::~MOS6510TestUndocumentedSBC()
{
}


// ============================================================================================
// SBC #Immediate - Binary
// ============================================================================================

void MOS6510TestUndocumentedSBC::testImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive")
        << quint8(0x30)
        << quint8(0x10)
        << quint8(0x01)
        << quint8(0x20)
        << quint8(0x35);

    QTest::newRow("borrow")
        << quint8(0x30)
        << quint8(0x40)
        << quint8(0x01)
        << quint8(0xF0)
        << quint8(0xB4);

    QTest::newRow("borrow with carry clear")
        << quint8(0x30)
        << quint8(0x10)
        << quint8(0x00)
        << quint8(0x1F)
        << quint8(0x35);

    QTest::newRow("zero")
        << quint8(0x40)
        << quint8(0x40)
        << quint8(0x01)
        << quint8(0x00)
        << quint8(0x37);

    QTest::newRow("negative")
        << quint8(0x10)
        << quint8(0x20)
        << quint8(0x01)
        << quint8(0xF0)
        << quint8(0xB4);

    QTest::newRow("overflow")
        << quint8(0x80)
        << quint8(0x01)
        << quint8(0x01)
        << quint8(0x7F)
        << quint8(0x75);

    QTest::newRow("overflow negative")
        << quint8(0x7F)
        << quint8(0xFF)
        << quint8(0x01)
        << quint8(0x80)
        << quint8(0xF4);

    QTest::newRow("borrow from zero")
        << quint8(0x00)
        << quint8(0x01)
        << quint8(0x01)
        << quint8(0xFF)
        << quint8(0xB4);
}

void MOS6510TestUndocumentedSBC::testImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(static_cast<quint8>(0x74 | carryIn));
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xEB);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);

    const quint8 initialStatus = m_cpu.status();

    // Cycle 1: Opcode fetch
    clock();
    verifyRead(0x1000, 0xEB);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: Read immediate operand and execute SBC
    clock();
    verifyRead(0x1001, operand);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    //
    // This proves that undocumented SBC $EB takes exactly two cycles.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), expectedStatus);
}


// ============================================================================================
// SBC #Immediate - Decimal
// ============================================================================================

void MOS6510TestUndocumentedSBC::testImmediateDecimal_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("00 - 00 borrow")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0)
        << quint8(0x99)
        << quint8(0xBC);

    QTest::newRow("00 - 00")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(1)
        << quint8(0x00)
        << quint8(0x3F);

    QTest::newRow("00 - 01")
        << quint8(0x00)
        << quint8(0x01)
        << quint8(1)
        << quint8(0x99)
        << quint8(0xBC);

    QTest::newRow("0A - 00")
        << quint8(0x0A)
        << quint8(0x00)
        << quint8(1)
        << quint8(0x0A)
        << quint8(0x3D);

    QTest::newRow("0B - 00 borrow")
        << quint8(0x0B)
        << quint8(0x00)
        << quint8(0)
        << quint8(0x0A)
        << quint8(0x3D);

    QTest::newRow("9A - 00")
        << quint8(0x9A)
        << quint8(0x00)
        << quint8(1)
        << quint8(0x9A)
        << quint8(0xBD);

    QTest::newRow("9B - 00 borrow")
        << quint8(0x9B)
        << quint8(0x00)
        << quint8(0)
        << quint8(0x9A)
        << quint8(0xBD);

    QTest::newRow("10 - 01")
        << quint8(0x10)
        << quint8(0x01)
        << quint8(1)
        << quint8(0x09)
        << quint8(0x3D);

    QTest::newRow("10 - 01 borrow")
        << quint8(0x10)
        << quint8(0x01)
        << quint8(0)
        << quint8(0x08)
        << quint8(0x3D);

    QTest::newRow("20 - 10")
        << quint8(0x20)
        << quint8(0x10)
        << quint8(1)
        << quint8(0x10)
        << quint8(0x3D);

    QTest::newRow("20 - 10 borrow")
        << quint8(0x20)
        << quint8(0x10)
        << quint8(0)
        << quint8(0x09)
        << quint8(0x3D);

    QTest::newRow("50 - 50")
        << quint8(0x50)
        << quint8(0x50)
        << quint8(1)
        << quint8(0x00)
        << quint8(0x3F);

    QTest::newRow("80 - 01")
        << quint8(0x80)
        << quint8(0x01)
        << quint8(1)
        << quint8(0x79)
        << quint8(0x7D);

    QTest::newRow("7F - FF")
        << quint8(0x7F)
        << quint8(0xFF)
        << quint8(1)
        << quint8(0x20)
        << quint8(0xFC);

    QTest::newRow("00 - 01 borrow")
        << quint8(0x00)
        << quint8(0x01)
        << quint8(0)
        << quint8(0x98)
        << quint8(0xBC);

    QTest::newRow("10 - 20")
        << quint8(0x10)
        << quint8(0x20)
        << quint8(1)
        << quint8(0x90)
        << quint8(0xBC);
}

void MOS6510TestUndocumentedSBC::testImmediateDecimal()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);

    quint8 initialStatus = 0x3C;
    if (carryIn != 0)
    {
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    }

    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xEB);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);

    // Cycle 1: Opcode fetch
    clock();
    verifyRead(0x1000, 0xEB);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: Read immediate operand and execute SBC in decimal mode
    clock();
    verifyRead(0x1001, operand);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    //
    // This proves that undocumented SBC $EB also takes exactly two cycles
    // in decimal mode.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), expectedStatus);
}