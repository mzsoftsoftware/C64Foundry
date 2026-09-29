#include "MOS6510TestUndocumentedISB.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedISB::MOS6510TestUndocumentedISB()
{
}

MOS6510TestUndocumentedISB::~MOS6510TestUndocumentedISB()
{
}


MOS6510TestUndocumentedISB::ISBResult
MOS6510TestUndocumentedISB::referenceISB(
    const quint8 accumulator,
    const quint8 memory,
    const bool carry,
    const bool decimal)
{
    ISBResult result;

    //
    // ISB/ISC first increments memory.
    //
    result.memory =
        static_cast<quint8>(memory + 1);

    const quint8 operand = result.memory;

    const quint16 borrow =
        carry ? 0 : 1;

    const quint16 binaryResult =
        static_cast<quint16>(
            accumulator)
        - static_cast<quint16>(
            operand)
        - borrow;

    const quint8 binaryResult8 =
        static_cast<quint8>(
            binaryResult);

    //
    // NMOS SBC derives N, Z and V from the binary result,
    // even when decimal mode is active.
    //
    result.zero =
        binaryResult8 == 0;

    result.negative =
        (binaryResult8 & 0x80) != 0;

    result.overflow =
        ((accumulator ^ binaryResult8)
         & (accumulator ^ operand)
         & 0x80) != 0;

    if (!decimal)
    {
        result.accumulator =
            binaryResult8;

        result.carry =
            binaryResult < 0x100;

        return result;
    }

    //
    // NMOS 6502 decimal SBC.
    //
    qint16 low =
        static_cast<qint16>(
            accumulator & 0x0F)
        - static_cast<qint16>(
            operand & 0x0F)
        - static_cast<qint16>(
            borrow);

    qint16 high =
        static_cast<qint16>(
            accumulator >> 4)
        - static_cast<qint16>(
            operand >> 4);

    if (low < 0)
    {
        low -= 6;
        --high;
    }

    if (high < 0)
    {
        high -= 6;
    }

    result.accumulator =
        static_cast<quint8>(
            ((static_cast<quint8>(high) << 4) & 0xF0)
            | (static_cast<quint8>(low) & 0x0F));

    result.carry =
        binaryResult < 0x100;

    return result;
}


quint8 MOS6510TestUndocumentedISB::expectedStatus(
    const quint8 initialStatus,
    const ISBResult& result)
{
    const quint8 affectedFlags =
        static_cast<quint8>(
            static_cast<quint8>(
                MOS6510StatusFlag::Carry)
            | static_cast<quint8>(
                MOS6510StatusFlag::Zero)
            | static_cast<quint8>(
                MOS6510StatusFlag::Overflow)
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

    if (result.overflow)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Overflow);
    }

    if (result.negative)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Negative);
    }

    return status;
}


void MOS6510TestUndocumentedISB::testZeroPage_data()
{
    QTest::addColumn<int>("accumulator");
    QTest::addColumn<int>("memory");
    QTest::addColumn<int>("initialStatus");

    //
    // Binary mode.
    //
    QTest::newRow("binary equal")
        << 0x40 << 0x3F << 0x21;

    QTest::newRow("binary no borrow")
        << 0x50 << 0x3F << 0x21;

    QTest::newRow("binary borrow")
        << 0x20 << 0x3F << 0x21;

    QTest::newRow("binary carry clear")
        << 0x40 << 0x3F << 0x20;

    QTest::newRow("binary memory wrap")
        << 0x00 << 0xFF << 0x21;

    QTest::newRow("binary negative")
        << 0x00 << 0x00 << 0x21;

    QTest::newRow("binary overflow")
        << 0x80 << 0x00 << 0x21;

    QTest::newRow("binary positive overflow")
        << 0x7F << 0xFE << 0x21;

    //
    // Decimal mode.
    //
    QTest::newRow("decimal simple")
        << 0x50 << 0x08 << 0x29;

    QTest::newRow("decimal low borrow")
        << 0x50 << 0x10 << 0x29;

    QTest::newRow("decimal borrow")
        << 0x00 << 0x00 << 0x29;

    QTest::newRow("decimal carry clear input")
        << 0x50 << 0x08 << 0x28;

    QTest::newRow("decimal memory wrap")
        << 0x00 << 0xFF << 0x29;

    //
    // I/B/U must survive. C/Z/V/N are deliberately overwritten.
    //
    QTest::newRow("unrelated flags preserved")
        << 0x40 << 0x3F << 0x3F;
}


void MOS6510TestUndocumentedISB::testZeroPage()
{
    QFETCH(int, accumulator);
    QFETCH(int, memory);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 initialAccumulator =
        static_cast<quint8>(accumulator);

    const quint8 initialMemory =
        static_cast<quint8>(memory);

    const quint8 status =
        static_cast<quint8>(initialStatus);

    const bool carry =
        (status
         & static_cast<quint8>(
             MOS6510StatusFlag::Carry))
        != 0;

    const bool decimal =
        (status
         & static_cast<quint8>(
             MOS6510StatusFlag::Decimal))
        != 0;

    const ISBResult expected =
        referenceISB(
            initialAccumulator,
            initialMemory,
            carry,
            decimal);

    const quint8 finalStatus =
        expectedStatus(
            status,
            expected);

    m_cpu.setAccumulator(initialAccumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xE7);
    m_memory.writeRAM(0x1001, 0x80);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0080, initialMemory);

    //
    // C1: Opcode fetch
    //
    clock();
    verifyRead(0x1000, 0xE7);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    //
    // C2: Zero-page address
    //
    clock();
    verifyRead(0x1001, 0x80);

    //
    // C3: Read old memory value
    //
    clock();
    verifyRead(0x0080, initialMemory);

    //
    // C4: NMOS RMW dummy write
    //
    clock();
    verifyWrite(0x0080, initialMemory);

    //
    // C5: Increment + final write + SBC
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
        quint8(0x22));

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x33));

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0x44));

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    //
    // C6: Next opcode proves exactly 5 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));
}


void MOS6510TestUndocumentedISB::testZeroPageX()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xF7);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F8 + $10 wraps to $08.
    //
    m_memory.writeRAM(0x0008, initialMemory);

    clock();
    verifyRead(0x1000, 0xF7);

    clock();
    verifyRead(0x1001, 0xF8);

    clock();
    verifyRead(
        0x00F8,
        m_memory.readRAM(0x00F8));

    clock();
    verifyRead(0x0008, initialMemory);

    clock();
    verifyWrite(0x0008, initialMemory);

    clock();
    verifyWrite(0x0008, expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x0008),
        expected.memory);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

    //
    // ISB zp,X = exactly 6 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedISB::testAbsolute()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xEF);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, initialMemory);

    clock();
    verifyRead(0x1000, 0xEF);

    clock();
    verifyRead(0x1001, 0x34);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1234, initialMemory);

    clock();
    verifyWrite(0x1234, initialMemory);

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

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

    //
    // ISB abs = exactly 6 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedISB::testAbsoluteX()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xFF);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xFF);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Fixed indexed dummy read.
    //
    clock();
    verifyRead(0x1230, initialMemory);

    //
    // Actual RMW read.
    //
    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, initialMemory);

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

    //
    // ISB abs,X = exactly 7 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedISB::testAbsoluteXPageCross()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xFF);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12F0 + $20 = $1310
    //
    m_memory.writeRAM(0x1310, initialMemory);

    clock();
    verifyRead(0x1000, 0xFF);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Wrong-page dummy read.
    //
    clock();
    verifyRead(
        0x1210,
        m_memory.readRAM(0x1210));

    clock();
    verifyRead(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, initialMemory);

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
    // Page crossing does not add another cycle.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedISB::testAbsoluteY()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xFB);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xFB);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, initialMemory);

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


void MOS6510TestUndocumentedISB::testAbsoluteYPageCross()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xFB);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1310, initialMemory);

    clock();
    verifyRead(0x1000, 0xFB);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(
        0x1210,
        m_memory.readRAM(0x1210));

    clock();
    verifyRead(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, initialMemory);

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


void MOS6510TestUndocumentedISB::testIndirectX()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xE3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20,X) -> pointer $30/$31 -> $1234
    //
    m_memory.writeRAM(0x0030, 0x34);
    m_memory.writeRAM(0x0031, 0x12);
    m_memory.writeRAM(0x1234, initialMemory);

    clock();
    verifyRead(0x1000, 0xE3);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(
        0x0020,
        m_memory.readRAM(0x0020));

    clock();
    verifyRead(0x0030, 0x34);

    clock();
    verifyRead(0x0031, 0x12);

    clock();
    verifyRead(0x1234, initialMemory);

    clock();
    verifyWrite(0x1234, initialMemory);

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

    //
    // ISB (zp,X) = exactly 8 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedISB::testIndirectXZeroPageWrap()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xE3);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F8 + $10 -> $08 inside zero page.
    //
    m_memory.writeRAM(0x0008, 0x34);
    m_memory.writeRAM(0x0009, 0x12);
    m_memory.writeRAM(0x1234, initialMemory);

    clock();
    verifyRead(0x1000, 0xE3);

    clock();
    verifyRead(0x1001, 0xF8);

    clock();
    verifyRead(
        0x00F8,
        m_memory.readRAM(0x00F8));

    clock();
    verifyRead(0x0008, 0x34);

    clock();
    verifyRead(0x0009, 0x12);

    clock();
    verifyRead(0x1234, initialMemory);

    clock();
    verifyWrite(0x1234, initialMemory);

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


void MOS6510TestUndocumentedISB::testIndirectY()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xF3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20),Y -> $1220 + $10 = $1230
    //
    m_memory.writeRAM(0x0020, 0x20);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xF3);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x20);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // Fixed indexed dummy read.
    //
    clock();
    verifyRead(0x1230, initialMemory);

    //
    // Actual RMW read.
    //
    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, initialMemory);

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

    //
    // ISB (zp),Y = exactly 8 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedISB::testIndirectYPageCross()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xF3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20),Y -> $12F0 + $20 = $1310
    //
    m_memory.writeRAM(0x0020, 0xF0);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1310, initialMemory);

    clock();
    verifyRead(0x1000, 0xF3);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xF0);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // Wrong-page dummy read.
    //
    clock();
    verifyRead(
        0x1210,
        m_memory.readRAM(0x1210));

    clock();
    verifyRead(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, initialMemory);

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
    // No extra page-cross cycle.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedISB::testIndirectYZeroPageWrap()
{
    setupCpu();

    const quint8 accumulator = 0x50;
    const quint8 initialMemory = 0x3F;
    const quint8 initialStatus = 0x21;

    const ISBResult expected =
        referenceISB(
            accumulator,
            initialMemory,
            true,
            false);

    //
    // Pointer high byte after zero-page wrap is read from $0000,
    // which is the MOS6510 data-direction register.
    //
    setDataDirectionRegister(0x12);

    //
    // setDataDirectionRegister() changes A and PC.
    // Restore the complete state required by this test.
    //
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xF3);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer high byte wraps from $FF to $00.
    //
    m_memory.writeRAM(0x00FF, 0x20);
    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xF3);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x20);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, initialMemory);

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


void MOS6510TestUndocumentedISB::testExhaustiveBinary()
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

                const quint8 initialMemory =
                    static_cast<quint8>(
                        memory);

                quint8 initialStatus =
                    static_cast<quint8>(
                        static_cast<quint8>(
                            MOS6510StatusFlag::Unused)
                        | static_cast<quint8>(
                            MOS6510StatusFlag::InterruptDisable)
                        | static_cast<quint8>(
                            MOS6510StatusFlag::Break));

                if (carry != 0)
                {
                    initialStatus |=
                        static_cast<quint8>(
                            MOS6510StatusFlag::Carry);
                }

                const ISBResult expected =
                    referenceISB(
                        initialAccumulator,
                        initialMemory,
                        carry != 0,
                        false);

                const quint8 finalStatus =
                    expectedStatus(
                        initialStatus,
                        expected);

                m_cpu.setAccumulator(
                    initialAccumulator);

                m_cpu.setStatus(
                    initialStatus);

                m_cpu.setProgramCounter(
                    0x1000);

                m_memory.writeRAM(
                    0x1000,
                    0xE7);

                m_memory.writeRAM(
                    0x1001,
                    0x80);

                m_memory.writeRAM(
                    0x0080,
                    initialMemory);

                //
                // ISB zp = 5 cycles.
                //
                clock();
                clock();
                clock();
                clock();
                clock();

                if (m_memory.readRAM(0x0080)
                        != expected.memory
                    || m_cpu.accumulator()
                           != expected.accumulator
                    || m_cpu.status()
                           != finalStatus)
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "ISB binary mismatch: "
                                "A=$%1 M=$%2 C=%3 "
                                "| actual A=$%4 M=$%5 P=$%6 "
                                "| expected A=$%7 M=$%8 P=$%9")
                                .arg(
                                    initialAccumulator,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    initialMemory,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(carry)
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


void MOS6510TestUndocumentedISB::testExhaustiveDecimal()
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

                const quint8 initialMemory =
                    static_cast<quint8>(
                        memory);

                quint8 initialStatus =
                    static_cast<quint8>(
                        static_cast<quint8>(
                            MOS6510StatusFlag::Unused)
                        | static_cast<quint8>(
                            MOS6510StatusFlag::Decimal)
                        | static_cast<quint8>(
                            MOS6510StatusFlag::InterruptDisable)
                        | static_cast<quint8>(
                            MOS6510StatusFlag::Break));

                if (carry != 0)
                {
                    initialStatus |=
                        static_cast<quint8>(
                            MOS6510StatusFlag::Carry);
                }

                const ISBResult expected =
                    referenceISB(
                        initialAccumulator,
                        initialMemory,
                        carry != 0,
                        true);

                const quint8 finalStatus =
                    expectedStatus(
                        initialStatus,
                        expected);

                m_cpu.setAccumulator(
                    initialAccumulator);

                m_cpu.setStatus(
                    initialStatus);

                m_cpu.setProgramCounter(
                    0x1000);

                m_memory.writeRAM(
                    0x1000,
                    0xE7);

                m_memory.writeRAM(
                    0x1001,
                    0x80);

                m_memory.writeRAM(
                    0x0080,
                    initialMemory);

                clock();
                clock();
                clock();
                clock();
                clock();

                if (m_memory.readRAM(0x0080)
                        != expected.memory
                    || m_cpu.accumulator()
                           != expected.accumulator
                    || m_cpu.status()
                           != finalStatus)
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "ISB decimal mismatch: "
                                "A=$%1 M=$%2 C=%3 "
                                "| actual A=$%4 M=$%5 P=$%6 "
                                "| expected A=$%7 M=$%8 P=$%9")
                                .arg(
                                    initialAccumulator,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    initialMemory,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(carry)
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
