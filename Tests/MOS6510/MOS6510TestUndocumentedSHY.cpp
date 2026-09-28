#include "MOS6510TestUndocumentedSHY.h"

#include <QtTest>


MOS6510TestUndocumentedSHY::MOS6510TestUndocumentedSHY()
{
}

MOS6510TestUndocumentedSHY::~MOS6510TestUndocumentedSHY()
{
}

void MOS6510TestUndocumentedSHY::testSHYAbsoluteX_data()
{
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint16>("expectedAddress");
    QTest::addColumn<quint8>("expectedValue");

    //
    // No page crossing.
    //
    QTest::newRow("basic")
        << quint16(0x1234)
        << quint8(0x10)
        << quint8(0xFF)
        << quint16(0x1244)
        << quint8(0x13);

    QTest::newRow("mask-y")
        << quint16(0x1234)
        << quint8(0x10)
        << quint8(0x0F)
        << quint16(0x1244)
        << quint8(0x03);

    QTest::newRow("zero-y")
        << quint16(0x1234)
        << quint8(0x10)
        << quint8(0x00)
        << quint16(0x1244)
        << quint8(0x00);

    QTest::newRow("different-high-byte")
        << quint16(0x5630)
        << quint8(0x20)
        << quint8(0xFF)
        << quint16(0x5650)
        << quint8(0x57);

    //
    // Page crossing.
    //
    // On the NMOS CPU, the stored value also replaces
    // the high byte of the write address.
    //
    QTest::newRow("page-cross")
        << quint16(0x12FF)
        << quint8(0x01)
        << quint8(0x0F)
        << quint16(0x0300)
        << quint8(0x03);

    QTest::newRow("page-cross-different-value")
        << quint16(0x12F0)
        << quint8(0x20)
        << quint8(0x05)
        << quint16(0x0110)
        << quint8(0x01);
}

void MOS6510TestUndocumentedSHY::testSHYAbsoluteX()
{
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, y);
    QFETCH(quint16, expectedAddress);
    QFETCH(quint8, expectedValue);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0xA5);
    m_cpu.setXRegister(x);
    m_cpu.setYRegister(y);
    m_cpu.setStackPointer(0x7E);
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9C);
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
    // SHY abs,X = 5 cycles.
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

    //
    // SHY must not modify registers other than memory.
    //
    QCOMPARE(m_cpu.accumulator(), quint8(0xA5));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));

    //
    // SHY does not modify flags.
    //
    QCOMPARE(m_cpu.status(), quint8(0xED));

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}

void MOS6510TestUndocumentedSHY::testSHYAbsoluteXFlags()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x5A);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0xFF);
    m_cpu.setStackPointer(0x81);

    //
    // Set a deliberately mixed status value.
    //
    m_cpu.setStatus(0xED);

    m_memory.writeRAM(0x1000, 0x9C);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);

    for (quint8 cycle = 0;
         cycle < 5;
         ++cycle)
    {
        clock();
    }

    QCOMPARE(m_cpu.status(), quint8(0xED));

    QCOMPARE(m_cpu.accumulator(), quint8(0x5A));
    QCOMPARE(m_cpu.xRegister(), quint8(0x10));
    QCOMPARE(m_cpu.yRegister(), quint8(0xFF));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x81));

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x13));
}

void MOS6510TestUndocumentedSHY::testSHYAbsoluteXCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0xFF);

    m_memory.writeRAM(0x1000, 0x9C);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // Effective address:
    //
    // $1234 + $10 = $1244
    //
    // Store value:
    //
    // Y & ($12 + 1)
    // $FF & $13 = $13
    //

    //
    // Cycle 1:
    // Fetch opcode.
    //
    clock();
    verifyRead(0x1000, 0x9C);

    //
    // Cycle 2:
    // Read low byte.
    //
    clock();
    verifyRead(0x1001, 0x34);

    //
    // Cycle 3:
    // Read high byte.
    //
    clock();
    verifyRead(0x1002, 0x12);

    //
    // Cycle 4:
    // Indexed dummy read.
    //
    clock();
    verifyRead(
        0x1244,
        m_memory.readRAM(0x1244));

    //
    // Cycle 5:
    // Store masked Y.
    //
    clock();
    verifyWrite(0x1244, 0x13);

    //
    // Cycle 6:
    // SHY must already be complete.
    // Fetch next opcode.
    //
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(
        m_memory.readRAM(0x1244),
        quint8(0x13));
}

void MOS6510TestUndocumentedSHY::testSHYAbsoluteXCyclesPageCrossing()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setXRegister(0x01);
    m_cpu.setYRegister(0x0F);

    m_memory.writeRAM(0x1000, 0x9C);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // Base address:
    //
    // $12FF
    //
    // Indexed address:
    //
    // $12FF + $01 = $1300
    //
    // Indexed dummy address:
    //
    // $1200
    //
    // Store value:
    //
    // Y & ($12 + 1)
    // $0F & $13 = $03
    //
    // Because a page boundary was crossed, the NMOS
    // behaviour replaces the high byte of the write
    // address with the stored value:
    //
    // normal address: $1300
    // actual address: $0300
    //

    //
    // Protect both possible destinations so that the
    // final state can distinguish the special NMOS
    // behaviour from a normal indexed store.
    //
    m_memory.writeRAM(0x0300, 0xAA);
    m_memory.writeRAM(0x1300, 0x55);

    //
    // Cycle 1:
    // Fetch opcode.
    //
    clock();
    verifyRead(0x1000, 0x9C);

    //
    // Cycle 2:
    // Read low byte.
    //
    clock();
    verifyRead(0x1001, 0xFF);

    //
    // Cycle 3:
    // Read high byte.
    //
    clock();
    verifyRead(0x1002, 0x12);

    //
    // Cycle 4:
    // Indexed dummy read using the uncorrected
    // high byte.
    //
    clock();
    verifyRead(
        0x1200,
        m_memory.readRAM(0x1200));

    //
    // Cycle 5:
    // Store to the NMOS-corrupted address.
    //
    clock();
    verifyWrite(0x0300, 0x03);

    //
    // The normal indexed destination must remain
    // untouched.
    //
    QCOMPARE(
        m_memory.readRAM(0x0300),
        quint8(0x03));

    QCOMPARE(
        m_memory.readRAM(0x1300),
        quint8(0x55));

    //
    // Cycle 6:
    // No additional page-crossing cycle.
    // Fetch next opcode.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}
