#include "MOS6510TestUndocumentedRLA.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedRLA::MOS6510TestUndocumentedRLA()
{
}

MOS6510TestUndocumentedRLA::~MOS6510TestUndocumentedRLA()
{
}


MOS6510TestUndocumentedRLA::RLAResult
MOS6510TestUndocumentedRLA::referenceRLA(
    const quint8 accumulator,
    const quint8 memory,
    const bool carry)
{
    RLAResult result;

    result.carry =
        (memory & 0x80) != 0;

    result.memory =
        static_cast<quint8>(
            (memory << 1)
            | (carry ? 0x01 : 0x00));

    result.accumulator =
        static_cast<quint8>(
            accumulator & result.memory);

    result.zero =
        result.accumulator == 0;

    result.negative =
        (result.accumulator & 0x80) != 0;

    return result;
}


quint8 MOS6510TestUndocumentedRLA::expectedStatus(
    const quint8 initialStatus,
    const RLAResult& result)
{
    const quint8 affectedFlags =
        static_cast<quint8>(
            static_cast<quint8>(
                MOS6510StatusFlag::Carry)
            | static_cast<quint8>(
                MOS6510StatusFlag::Zero)
            | static_cast<quint8>(
                MOS6510StatusFlag::Negative));

    quint8 status =
        static_cast<quint8>(
            initialStatus & ~affectedFlags);

    if (result.carry)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Carry);
    }

    if (result.zero)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Zero);
    }

    if (result.negative)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Negative);
    }

    return status;
}


void MOS6510TestUndocumentedRLA::testZeroPage_data()
{
    QTest::addColumn<int>("accumulator");
    QTest::addColumn<int>("memory");
    QTest::addColumn<bool>("carry");
    QTest::addColumn<int>("initialStatus");

    QTest::newRow("zero")
        << 0x00 << 0x00 << false << 0x20;

    QTest::newRow("carry input zero")
        << 0xFF << 0x00 << true << 0x20;

    QTest::newRow("rotate without carry")
        << 0xFF << 0x01 << false << 0x20;

    QTest::newRow("rotate with carry")
        << 0xFF << 0x01 << true << 0x20;

    QTest::newRow("carry output")
        << 0xFF << 0x80 << false << 0x20;

    QTest::newRow("carry input and output")
        << 0xFF << 0x80 << true << 0x20;

    QTest::newRow("and produces zero")
        << 0x0F << 0x08 << false << 0x20;

    QTest::newRow("negative")
        << 0x80 << 0xC0 << false << 0x20;

    QTest::newRow("old carry cleared")
        << 0xFF << 0x01 << true << 0x21;

    QTest::newRow("old zero cleared")
        << 0xFF << 0x01 << false << 0x22;

    QTest::newRow("old negative cleared")
        << 0x7F << 0x01 << false << 0xA0;

    //
    // V, D, I, B and U must survive RLA.
    //
    QTest::newRow("unrelated flags preserved")
        << 0xFF << 0x01 << false << 0x7C;
}


void MOS6510TestUndocumentedRLA::testZeroPage()
{
    QFETCH(int, accumulator);
    QFETCH(int, memory);
    QFETCH(bool, carry);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 initialAccumulator =
        static_cast<quint8>(accumulator);

    const quint8 oldMemory =
        static_cast<quint8>(memory);

    quint8 status =
        static_cast<quint8>(initialStatus);

    if (carry)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Carry);
    }
    else
    {
        status &=
            static_cast<quint8>(
                ~static_cast<quint8>(
                    MOS6510StatusFlag::Carry));
    }

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            carry);

    const quint8 finalStatus =
        expectedStatus(
            status,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x33);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x27);
    m_memory.writeRAM(0x1001, 0x80);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0080, oldMemory);

    //
    // C1: Opcode
    //
    clock();
    verifyRead(0x1000, 0x27);

    //
    // C2: Zero-page address
    //
    clock();
    verifyRead(0x1001, 0x80);

    //
    // C3: Read old memory
    //
    clock();
    verifyRead(0x0080, oldMemory);

    //
    // C4: NMOS RMW dummy write of old value
    //
    clock();
    verifyWrite(0x0080, oldMemory);

    QCOMPARE(
        m_memory.readRAM(0x0080),
        oldMemory);

    //
    // C5: Write rotated value and finish RLA
    //
    clock();
    verifyWrite(0x0080, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x0080),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x33));

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x44));

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0x55));

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    //
    // C6 proves exactly 5 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testZeroPageX()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0x41;
    const bool carry = true;
    const quint8 initialStatus = 0x21;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            carry);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x37);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x5A);
    m_memory.writeRAM(0x0030, oldMemory);

    clock();
    verifyRead(0x1000, 0x37);

    clock();
    verifyRead(0x1001, 0x20);

    //
    // Indexed zero-page dummy read.
    //
    clock();
    verifyRead(0x0020, 0x5A);

    clock();
    verifyRead(0x0030, oldMemory);

    clock();
    verifyWrite(0x0030, oldMemory);

    clock();
    verifyWrite(0x0030, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x0030),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x10));

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x44));

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0x55));

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testZeroPageXWrap()
{
    setupCpu();

    const quint8 initialAccumulator = 0xFF;
    const quint8 oldMemory = 0x40;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            false);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x37);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x00F8, 0x5A);
    m_memory.writeRAM(0x0008, oldMemory);

    clock();
    verifyRead(0x1000, 0x37);

    clock();
    verifyRead(0x1001, 0xF8);

    clock();
    verifyRead(0x00F8, 0x5A);

    clock();
    verifyRead(0x0008, oldMemory);

    clock();
    verifyWrite(0x0008, oldMemory);

    clock();
    verifyWrite(0x0008, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x0008),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testAbsolute()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0x41;
    const quint8 initialStatus = 0x21;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            true);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x33);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x2F);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, oldMemory);

    clock();
    verifyRead(0x1000, 0x2F);

    clock();
    verifyRead(0x1001, 0x34);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1234),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedRLA::testAbsoluteX()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0x41;
    const quint8 initialStatus = 0x21;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            true);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x3F);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $1220 + X($10) = $1230.
    // For indexed RMW, C4 is an unconditional dummy read.
    // Without page crossing it is the effective address itself.
    //
    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x3F);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1230),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedRLA::testAbsoluteXPageCross()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0xC1;
    const quint8 initialStatus = 0x20;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x3F);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12F0 + $20 = $1310
    // intermediate/wrong-page address = $1210
    //
    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, oldMemory);

    clock();
    verifyRead(0x1000, 0x3F);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1210, 0x5A);

    clock();
    verifyRead(0x1310, oldMemory);

    clock();
    verifyWrite(0x1310, oldMemory);

    clock();
    verifyWrite(0x1310, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1310),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    //
    // Still exactly 7 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedRLA::testAbsoluteY()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0x41;
    const quint8 initialStatus = 0x21;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            true);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x33);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x3B);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x3B);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1230),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedRLA::testAbsoluteYPageCross()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0xC1;
    const quint8 initialStatus = 0x20;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x3B);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, oldMemory);

    clock();
    verifyRead(0x1000, 0x3B);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1210, 0x5A);

    clock();
    verifyRead(0x1310, oldMemory);

    clock();
    verifyWrite(0x1310, oldMemory);

    clock();
    verifyWrite(0x1310, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1310),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedRLA::testIndirectX()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0x41;
    const quint8 initialStatus = 0x21;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            true);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x23);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x5A);
    m_memory.writeRAM(0x0030, 0x34);
    m_memory.writeRAM(0x0031, 0x12);
    m_memory.writeRAM(0x1234, oldMemory);

    clock();
    verifyRead(0x1000, 0x23);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x5A);

    clock();
    verifyRead(0x0030, 0x34);

    clock();
    verifyRead(0x0031, 0x12);

    clock();
    verifyRead(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1234),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testIndirectXZeroPageWrap()
{
    setupCpu();

    const quint8 initialAccumulator = 0xFF;
    const quint8 oldMemory = 0x40;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            false);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x01);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x23);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $FE + X($01) = $FF.
    // Pointer high byte must wrap to $00.
    //
    m_memory.writeRAM(0x00FE, 0x5A);
    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x0000, 0x12);
    m_memory.writeRAM(0x1234, oldMemory);

    clock();
    verifyRead(0x1000, 0x23);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x5A);

    clock();
    verifyRead(0x00FF, 0x34);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1234),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testIndirectY()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0x41;
    const quint8 initialStatus = 0x21;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            true);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x33);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x33);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x20);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x33);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x20);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // Fixed RMW dummy read.
    // Without page crossing it equals EA.
    //
    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1230),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testIndirectYPageCross()
{
    setupCpu();

    const quint8 initialAccumulator = 0xF3;
    const quint8 oldMemory = 0xC1;
    const quint8 initialStatus = 0x20;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x33);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer = $12F0
    // + Y($20) = $1310
    // intermediate address = $1210
    //
    m_memory.writeRAM(0x0020, 0xF0);
    m_memory.writeRAM(0x0021, 0x12);

    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, oldMemory);

    clock();
    verifyRead(0x1000, 0x33);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xF0);

    clock();
    verifyRead(0x0021, 0x12);

    clock();
    verifyRead(0x1210, 0x5A);

    clock();
    verifyRead(0x1310, oldMemory);

    clock();
    verifyWrite(0x1310, oldMemory);

    clock();
    verifyWrite(0x1310, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1310),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    //
    // Still exactly 8 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testIndirectYZeroPageWrap()
{
    setupCpu();

    const quint8 initialAccumulator = 0xFF;
    const quint8 oldMemory = 0x40;

    const RLAResult expected =
        referenceRLA(
            initialAccumulator,
            oldMemory,
            false);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x33);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer low at $FF, high at $00.
    // Base = $1220, +Y = $1230.
    //
    m_memory.writeRAM(0x00FF, 0x20);
    m_memory.writeRAM(0x0000, 0x12);
    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x33);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x20);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x1230),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedRLA::testExhaustive()
{
    quint32 testCount = 0;

    for (quint16 accumulator = 0;
         accumulator <= 0xFF;
         ++accumulator)
    {
        for (quint16 memory = 0;
             memory <= 0xFF;
             ++memory)
        {
            for (quint8 carry = 0;
                 carry <= 1;
                 ++carry)
            {
                setupCpu();

                const quint8 initialAccumulator =
                    static_cast<quint8>(
                        accumulator);

                const quint8 oldMemory =
                    static_cast<quint8>(
                        memory);

                //
                // Set all unrelated flags so we also
                // verify their preservation.
                //
                quint8 initialStatus = 0xFC;

                if (carry != 0)
                {
                    initialStatus |=
                        static_cast<quint8>(
                            MOS6510StatusFlag::Carry);
                }

                const RLAResult expected =
                    referenceRLA(
                        initialAccumulator,
                        oldMemory,
                        carry != 0);

                const quint8 finalStatus =
                    expectedStatus(
                        initialStatus,
                        expected);

                m_cpu.setAccumulator(
                    initialAccumulator);

                m_cpu.setXRegister(0x33);
                m_cpu.setYRegister(0x44);
                m_cpu.setStackPointer(0x55);
                m_cpu.setStatus(initialStatus);
                m_cpu.setProgramCounter(0x1000);

                m_memory.writeRAM(
                    0x1000,
                    0x27);

                m_memory.writeRAM(
                    0x1001,
                    0x80);

                m_memory.writeRAM(
                    0x0080,
                    oldMemory);

                //
                // RLA zp = exactly 5 cycles.
                //
                clock();
                clock();
                clock();
                clock();
                clock();

                if (m_cpu.accumulator()
                        != expected.accumulator
                    || m_memory.readRAM(0x0080)
                           != expected.memory
                    || m_cpu.status()
                           != finalStatus
                    || m_cpu.xRegister()
                           != 0x33
                    || m_cpu.yRegister()
                           != 0x44
                    || m_cpu.stackPointer()
                           != 0x55)
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "RLA mismatch: "
                                "A=$%1 M=$%2 C=%3 "
                                "| actual A=$%4 M=$%5 P=$%6 "
                                "| expected A=$%7 M=$%8 P=$%9")
                                .arg(
                                    initialAccumulator,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    oldMemory,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    carry)
                                .arg(
                                    m_cpu.accumulator(),
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    m_memory.readRAM(0x0080),
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    m_cpu.status(),
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    expected.accumulator,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    expected.memory,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    finalStatus,
                                    2,
                                    16,
                                    QLatin1Char('0'))));
                }

                ++testCount;
            }
        }
    }

    QCOMPARE(
        testCount,
        quint32(256 * 256 * 2));
}
