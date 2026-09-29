#include "MOS6510TestUndocumentedTAS.h"

#include <QtTest>


MOS6510TestUndocumentedTAS::MOS6510TestUndocumentedTAS()
{
}

MOS6510TestUndocumentedTAS::~MOS6510TestUndocumentedTAS()
{
}

void MOS6510TestUndocumentedTAS::testTASAbsoluteY_data()
{
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("a");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("initialStackPointer");
    QTest::addColumn<quint16>("expectedAddress");
    QTest::addColumn<quint8>("expectedStackPointer");
    QTest::addColumn<quint8>("expectedValue");

    //
    // No page crossing.
    //
    // SP = A & X
    // value = SP & (baseHigh + 1)
    //
    QTest::newRow("basic")
        << quint16(0x1234)
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0x10)
        << quint8(0x55)
        << quint16(0x1244)
        << quint8(0xFF)
        << quint8(0x13);

    QTest::newRow("mask-a")
        << quint16(0x1234)
        << quint8(0x0F)
        << quint8(0xFF)
        << quint8(0x10)
        << quint8(0x55)
        << quint16(0x1244)
        << quint8(0x0F)
        << quint8(0x03);

    QTest::newRow("mask-x")
        << quint16(0x1234)
        << quint8(0xFF)
        << quint8(0x07)
        << quint8(0x10)
        << quint8(0x55)
        << quint16(0x1244)
        << quint8(0x07)
        << quint8(0x03);

    QTest::newRow("a-and-x")
        << quint16(0x5630)
        << quint8(0x7F)
        << quint8(0x5F)
        << quint8(0x20)
        << quint8(0xAA)
        << quint16(0x5650)
        << quint8(0x5F)
        << quint8(0x57);

    QTest::newRow("zero")
        << quint16(0x1234)
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0x10)
        << quint8(0x55)
        << quint16(0x1244)
        << quint8(0x00)
        << quint8(0x00);

    //
    // Page crossing.
    //
    QTest::newRow("page-cross")
        << quint16(0x12FF)
        << quint8(0x0F)
        << quint8(0x07)
        << quint8(0x01)
        << quint8(0xAA)
        << quint16(0x0300)
        << quint8(0x07)
        << quint8(0x03);

    QTest::newRow("page-cross-different-value")
        << quint16(0x12F0)
        << quint8(0x07)
        << quint8(0x05)
        << quint8(0x20)
        << quint8(0xAA)
        << quint16(0x0110)
        << quint8(0x05)
        << quint8(0x01);
}

void MOS6510TestUndocumentedTAS::testTASAbsoluteY()
{
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, a);
    QFETCH(quint8, x);
    QFETCH(quint8, y);
    QFETCH(quint8, initialStackPointer);
    QFETCH(quint16, expectedAddress);
    QFETCH(quint8, expectedStackPointer);
    QFETCH(quint8, expectedValue);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(a);
    m_cpu.setXRegister(x);
    m_cpu.setYRegister(y);
    m_cpu.setStackPointer(initialStackPointer);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9B);

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
    // TAS abs,Y = 5 cycles.
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

    QCOMPARE(
        m_cpu.stackPointer(),
        expectedStackPointer);

    //
    // A, X and Y are unchanged.
    //
    QCOMPARE(m_cpu.accumulator(), a);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.yRegister(), y);

    //
    // TAS does not modify flags.
    //
    QCOMPARE(m_cpu.status(), quint8(0xED));

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));
}

void MOS6510TestUndocumentedTAS::testTASRegistersAndFlags()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    m_cpu.setAccumulator(0x5A);
    m_cpu.setXRegister(0x3C);
    m_cpu.setYRegister(0x10);

    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9B);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);

    //
    // A & X:
    //
    // $5A = 01011010
    // $3C = 00111100
    //       --------
    //       00011000 = $18
    //
    // SP = $18
    //
    // Store value:
    //
    // $18 & ($12 + 1)
    // $18 & $13
    // = $10
    //
    for (quint8 cycle = 0;
         cycle < 5;
         ++cycle)
    {
        clock();
    }

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0x18));

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x10));

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x5A));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x3C));

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x10));

    QCOMPARE(
        m_cpu.status(),
        quint8(0xED));
}

void MOS6510TestUndocumentedTAS::testTASAbsoluteYCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    m_cpu.setAccumulator(0xFF);
    m_cpu.setXRegister(0xFF);
    m_cpu.setYRegister(0x10);

    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9B);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // Base address:
    //
    // $1234 + Y($10) = $1244
    //
    // SP = A & X
    //    = $FF
    //
    // value:
    //
    // SP & ($12 + 1)
    // $FF & $13
    // = $13
    //

    clock();
    verifyRead(0x1000, 0x9B);

    clock();
    verifyRead(0x1001, 0x34);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Fixed indexed-store dummy read.
    //
    clock();
    verifyRead(
        0x1244,
        m_memory.readRAM(0x1244));

    clock();
    verifyWrite(0x1244, 0x13);

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0xFF));

    QCOMPARE(
        m_cpu.status(),
        quint8(0xED));

    //
    // Exactly 5 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestUndocumentedTAS::testTASAbsoluteYCyclesPageCrossing()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    m_cpu.setAccumulator(0x0F);
    m_cpu.setXRegister(0x07);
    m_cpu.setYRegister(0x01);

    m_cpu.setStackPointer(0xAA);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9B);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12FF + Y($01) = $1300
    //
    // SP = A & X
    //
    // $0F & $07 = $07
    //
    // value:
    //
    // $07 & ($12 + 1)
    // $07 & $13
    // = $03
    //
    // Because the page is crossed:
    //
    // dummy read   -> $1200
    // actual write -> $0300
    //

    m_memory.writeRAM(0x0300, 0xAA);
    m_memory.writeRAM(0x1300, 0x55);

    clock();
    verifyRead(0x1000, 0x9B);

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
        m_cpu.stackPointer(),
        quint8(0x07));

    //
    // Make sure the normal effective address
    // was NOT written.
    //
    QCOMPARE(
        m_memory.readRAM(0x1300),
        quint8(0x55));

    QCOMPARE(
        m_memory.readRAM(0x0300),
        quint8(0x03));

    //
    // Registers other than SP and all flags
    // remain unchanged.
    //
    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x0F));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x07));

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x01));

    QCOMPARE(
        m_cpu.status(),
        quint8(0xED));

    //
    // No additional page-crossing cycle.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}
