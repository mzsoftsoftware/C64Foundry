#include "MOS6510TestUndocumentedAHX.h"

#include <QtTest>


MOS6510TestUndocumentedAHX::MOS6510TestUndocumentedAHX()
{
}

MOS6510TestUndocumentedAHX::~MOS6510TestUndocumentedAHX()
{
}

void MOS6510TestUndocumentedAHX::testAHXAbsoluteY_data()
{
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("a");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint16>("expectedAddress");
    QTest::addColumn<quint8>("expectedValue");

    //
    // No page crossing.
    //
    QTest::newRow("basic")
        << quint16(0x1234)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x13);

    QTest::newRow("mask-a")
        << quint16(0x1234)
        << quint8(0x0F)
        << quint8(0xFF)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x03);

    QTest::newRow("mask-x")
        << quint16(0x1234)
        << quint8(0xFF)
        << quint8(0x05)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x01);

    QTest::newRow("mask-a-and-x")
        << quint16(0x1234)
        << quint8(0x0F)
        << quint8(0x07)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x03);

    QTest::newRow("zero")
        << quint16(0x1234)
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x00);

    QTest::newRow("different-high-byte")
        << quint16(0x5630)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x20)
        << quint16(0x5650)
        << quint8(0x57);

    //
    // Page crossing.
    //
    QTest::newRow("page-cross")
        << quint16(0x12FF)
        << quint8(0x0F)
        << quint8(0x07)
        << quint8(0x01)
        << quint16(0x0300)
        << quint8(0x03);

    QTest::newRow("page-cross-different-value")
        << quint16(0x12F0)
        << quint8(0x07)
        << quint8(0x05)
        << quint8(0x20)
        << quint16(0x0110)
        << quint8(0x01);
}

void MOS6510TestUndocumentedAHX::testAHXAbsoluteY()
{
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, a);
    QFETCH(quint8, x);
    QFETCH(quint8, y);
    QFETCH(quint16, expectedAddress);
    QFETCH(quint8, expectedValue);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(a);
    m_cpu.setXRegister(x);
    m_cpu.setYRegister(y);
    m_cpu.setStackPointer(0x7E);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9F);
    m_memory.writeRAM(
        0x1001,
        static_cast<quint8>(
            baseAddress & 0x00FF));
    m_memory.writeRAM(
        0x1002,
        static_cast<quint8>(
            baseAddress >> 8));

    m_memory.writeRAM(expectedAddress, 0xAA);

    //
    // AHX abs,Y = 5 cycles.
    //
    for (quint8 cycle = 0;
         cycle < 5;
         ++cycle)
    {
        clock();
    }

    QCOMPARE(
        m_memory.readRAM(expectedAddress),
        expectedValue);

    QCOMPARE(m_cpu.accumulator(), a);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));
    QCOMPARE(m_cpu.status(), quint8(0xED));

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestUndocumentedAHX::testAHXIndirectY_data()
{
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("a");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint16>("expectedAddress");
    QTest::addColumn<quint8>("expectedValue");

    //
    // No page crossing.
    //
    QTest::newRow("basic")
        << quint16(0x1234)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x13);

    QTest::newRow("mask-a")
        << quint16(0x1234)
        << quint8(0x0F)
        << quint8(0xFF)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x03);

    QTest::newRow("mask-x")
        << quint16(0x1234)
        << quint8(0xFF)
        << quint8(0x05)
        << quint8(0x10)
        << quint16(0x1244)
        << quint8(0x01);

    //
    // Page crossing.
    //
    QTest::newRow("page-cross")
        << quint16(0x12FF)
        << quint8(0x0F)
        << quint8(0x07)
        << quint8(0x01)
        << quint16(0x0300)
        << quint8(0x03);

    QTest::newRow("page-cross-different-value")
        << quint16(0x12F0)
        << quint8(0x07)
        << quint8(0x05)
        << quint8(0x20)
        << quint16(0x0110)
        << quint8(0x01);
}

void MOS6510TestUndocumentedAHX::testAHXIndirectY()
{
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, a);
    QFETCH(quint8, x);
    QFETCH(quint8, y);
    QFETCH(quint16, expectedAddress);
    QFETCH(quint8, expectedValue);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(a);
    m_cpu.setXRegister(x);
    m_cpu.setYRegister(y);
    m_cpu.setStackPointer(0x7E);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x93);
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(
        0x0020,
        static_cast<quint8>(
            baseAddress & 0x00FF));

    m_memory.writeRAM(
        0x0021,
        static_cast<quint8>(
            baseAddress >> 8));

    m_memory.writeRAM(expectedAddress, 0xAA);

    //
    // AHX (zp),Y = 6 cycles.
    //
    for (quint8 cycle = 0;
         cycle < 6;
         ++cycle)
    {
        clock();
    }

    QCOMPARE(
        m_memory.readRAM(expectedAddress),
        expectedValue);

    QCOMPARE(m_cpu.accumulator(), a);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));
    QCOMPARE(m_cpu.status(), quint8(0xED));

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}

void MOS6510TestUndocumentedAHX::testAHXRegistersAndFlags()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x0F);
    m_cpu.setXRegister(0x07);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(0x81);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9F);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);

    for (quint8 cycle = 0;
         cycle < 5;
         ++cycle)
    {
        clock();
    }

    QCOMPARE(m_cpu.accumulator(), quint8(0x0F));
    QCOMPARE(m_cpu.xRegister(), quint8(0x07));
    QCOMPARE(m_cpu.yRegister(), quint8(0x10));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x81));
    QCOMPARE(m_cpu.status(), quint8(0xED));

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x03));
}

void MOS6510TestUndocumentedAHX::testAHXAbsoluteYCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0xFF);
    m_cpu.setYRegister(0x10);

    m_memory.writeRAM(0x1000, 0x9F);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $1234 + Y($10) = $1244
    //
    // value:
    //
    // A & X & ($12 + 1)
    // $FF & $FF & $13 = $13
    //

    clock();
    verifyRead(0x1000, 0x9F);

    clock();
    verifyRead(0x1001, 0x34);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(
        0x1244,
        m_memory.readRAM(0x1244));

    clock();
    verifyWrite(0x1244, 0x13);

    //
    // Exactly 5 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x13));
}

void MOS6510TestUndocumentedAHX::testAHXAbsoluteYCyclesPageCrossing()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x0F);
    m_cpu.setXRegister(0x07);
    m_cpu.setYRegister(0x01);

    m_memory.writeRAM(0x1000, 0x9F);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12FF + Y($01) = $1300
    //
    // value:
    //
    // $0F & $07 & ($12 + 1)
    // = $03
    //
    // Page crossing:
    //
    // dummy read  -> $1200
    // actual write -> $0300
    //

    m_memory.writeRAM(0x0300, 0xAA);
    m_memory.writeRAM(0x1300, 0x55);

    clock();
    verifyRead(0x1000, 0x9F);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(
        0x1200,
        m_memory.readRAM(0x1200));

    clock();
    verifyWrite(0x0300, 0x03);

    QCOMPARE(
        m_memory.readRAM(0x0300),
        quint8(0x03));

    QCOMPARE(
        m_memory.readRAM(0x1300),
        quint8(0x55));

    //
    // No extra page-crossing cycle.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestUndocumentedAHX::testAHXIndirectYCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0xFF);
    m_cpu.setYRegister(0x10);

    m_memory.writeRAM(0x1000, 0x93);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x34);
    m_memory.writeRAM(0x0021, 0x12);

    //
    // ($20) = $1234
    // $1234 + Y($10) = $1244
    //

    clock();
    verifyRead(0x1000, 0x93);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x34);

    clock();
    verifyRead(0x0021, 0x12);

    clock();
    verifyRead(
        0x1244,
        m_memory.readRAM(0x1244));

    clock();
    verifyWrite(0x1244, 0x13);

    //
    // Exactly 6 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x13));
}

void MOS6510TestUndocumentedAHX::testAHXIndirectYCyclesPageCrossing()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x0F);
    m_cpu.setXRegister(0x07);
    m_cpu.setYRegister(0x01);

    m_memory.writeRAM(0x1000, 0x93);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0xFF);
    m_memory.writeRAM(0x0021, 0x12);

    m_memory.writeRAM(0x0300, 0xAA);
    m_memory.writeRAM(0x1300, 0x55);

    //
    // ($20) = $12FF
    //
    // $12FF + Y($01) = $1300
    //
    // value = $0F & $07 & $13
    //       = $03
    //
    // dummy read   $1200
    // actual write $0300
    //

    clock();
    verifyRead(0x1000, 0x93);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFF);

    clock();
    verifyRead(0x0021, 0x12);

    clock();
    verifyRead(
        0x1200,
        m_memory.readRAM(0x1200));

    clock();
    verifyWrite(0x0300, 0x03);

    QCOMPARE(
        m_memory.readRAM(0x0300),
        quint8(0x03));

    QCOMPARE(
        m_memory.readRAM(0x1300),
        quint8(0x55));

    //
    // No additional page-crossing cycle.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestUndocumentedAHX::testAHXIndirectYZeroPageWrap()
{
    setupCpu();

    //
    // Pointer high byte after zero-page wrap is read from $0000,
    // which is the MOS6510 data-direction register.
    //
    setDataDirectionRegister(0x12);

    //
    // Restore the CPU state required by this test.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0xFF);
    m_cpu.setYRegister(0x10);

    m_memory.writeRAM(0x1000, 0x93);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // The pointer starts at $FF.
    //
    // Its high byte must wrap inside zero page:
    //
    // low  = [$00FF] = $34
    // high = [$0000] = $12
    //
    // NOT [$0100].
    //
    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x0100, 0x56);

    //
    // Effective address:
    //
    // $1234 + $10 = $1244
    //
    m_memory.writeRAM(0x1244, 0xAA);

    clock();
    verifyRead(0x1000, 0x93);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x34);

    //
    // Critical zero-page wrap.
    //
    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(
        0x1244,
        m_memory.readRAM(0x1244));

    clock();
    verifyWrite(0x1244, 0x13);

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x13));

    clock();
    verifyRead(0x1002, 0xEA);
}
