#include "MOS6510TestUndocumentedDCP.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedDCP::MOS6510TestUndocumentedDCP()
{
}

MOS6510TestUndocumentedDCP::~MOS6510TestUndocumentedDCP()
{
}


MOS6510TestUndocumentedDCP::DCPResult
MOS6510TestUndocumentedDCP::referenceDCP(
    const quint8 accumulator,
    const quint8 memory)
{
    DCPResult result;

    result.memory =
        static_cast<quint8>(memory - 1);

    const quint8 compareResult =
        static_cast<quint8>(
            accumulator - result.memory);

    result.carry =
        accumulator >= result.memory;

    result.zero =
        accumulator == result.memory;

    result.negative =
        (compareResult & 0x80) != 0;

    return result;
}


quint8 MOS6510TestUndocumentedDCP::expectedStatus(
    const quint8 initialStatus,
    const DCPResult& result)
{
    const quint8 affectedFlags =
        static_cast<quint8>(
            static_cast<quint8>(MOS6510StatusFlag::Carry)
            | static_cast<quint8>(MOS6510StatusFlag::Zero)
            | static_cast<quint8>(MOS6510StatusFlag::Negative));

    quint8 status =
        static_cast<quint8>(
            initialStatus & ~affectedFlags);

    if (result.carry)
    {
        status |=
            static_cast<quint8>(MOS6510StatusFlag::Carry);
    }

    if (result.zero)
    {
        status |=
            static_cast<quint8>(MOS6510StatusFlag::Zero);
    }

    if (result.negative)
    {
        status |=
            static_cast<quint8>(MOS6510StatusFlag::Negative);
    }

    return status;
}


void MOS6510TestUndocumentedDCP::testZeroPage_data()
{
    QTest::addColumn<int>("accumulator");
    QTest::addColumn<int>("memory");
    QTest::addColumn<int>("initialStatus");

    QTest::newRow("equal after decrement")
        << 0x40 << 0x41 << 0x20;

    QTest::newRow("accumulator greater")
        << 0x80 << 0x41 << 0x20;

    QTest::newRow("accumulator smaller")
        << 0x20 << 0x41 << 0x20;

    QTest::newRow("memory underflow")
        << 0x80 << 0x00 << 0x20;

    QTest::newRow("zero result")
        << 0x00 << 0x01 << 0x20;

    QTest::newRow("negative compare")
        << 0x00 << 0x80 << 0x20;

    QTest::newRow("old flags overwritten")
        << 0x40 << 0x41 << 0xA3;

    QTest::newRow("overflow preserved")
        << 0x40 << 0x41 << 0x60;

    QTest::newRow("decimal preserved")
        << 0x40 << 0x41 << 0x28;

    QTest::newRow("unrelated flags preserved")
        << 0x40 << 0x41 << 0x7C;
}


void MOS6510TestUndocumentedDCP::testZeroPage()
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

    const DCPResult expected =
        referenceDCP(
            initialAccumulator,
            initialMemory);

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

    m_memory.writeRAM(0x1000, 0xC7);
    m_memory.writeRAM(0x1001, 0x80);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0080, initialMemory);

    //
    // C1: Opcode
    //
    clock();
    verifyRead(0x1000, 0xC7);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), initialAccumulator);
    QCOMPARE(m_cpu.status(), status);

    //
    // C2: Zero-page address
    //
    clock();
    verifyRead(0x1001, 0x80);

    //
    // C3: Read old value
    //
    clock();
    verifyRead(0x0080, initialMemory);

    //
    // C4: Dummy write old value
    //
    clock();
    verifyWrite(0x0080, initialMemory);

    //
    // C5: Write decremented value and perform comparison
    //
    clock();
    verifyWrite(0x0080, expected.memory);

    QCOMPARE(m_memory.readRAM(0x0080), expected.memory);

    QCOMPARE(m_cpu.accumulator(), initialAccumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), finalStatus);
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    //
    // C6: Next opcode
    //
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), initialAccumulator);
    QCOMPARE(m_cpu.status(), finalStatus);
}


void MOS6510TestUndocumentedDCP::testZeroPageX()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xD7);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F8 + $10 wraps to $08.
    //
    m_memory.writeRAM(0x0008, initialMemory);

    clock();
    verifyRead(0x1000, 0xD7);

    clock();
    verifyRead(0x1001, 0xF8);

    //
    // Indexed zero-page instructions perform the dummy read
    // from the unindexed zero-page address.
    //
    clock();
    verifyRead(0x00F8, m_memory.readRAM(0x00F8));

    clock();
    verifyRead(0x0008, initialMemory);

    clock();
    verifyWrite(0x0008, initialMemory);

    clock();
    verifyWrite(0x0008, expected.memory);

    QCOMPARE(m_memory.readRAM(0x0008), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    //
    // DCP zp,X = exactly 6 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}


void MOS6510TestUndocumentedDCP::testAbsolute()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xCF);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, initialMemory);

    clock();
    verifyRead(0x1000, 0xCF);

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

    QCOMPARE(m_memory.readRAM(0x1234), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    //
    // DCP abs = exactly 6 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}


void MOS6510TestUndocumentedDCP::testAbsoluteX()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xDF);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xDF);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Indexed RMW always performs an indexed dummy read.
    //
    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, expected.memory);

    QCOMPARE(m_memory.readRAM(0x1230), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    //
    // DCP abs,X = exactly 7 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedDCP::testAbsoluteXPageCross()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xDF);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12F0 + $20 = $1310
    //
    m_memory.writeRAM(0x1310, initialMemory);

    clock();
    verifyRead(0x1000, 0xDF);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Wrong-page dummy read:
    // $12F0 + $20 -> temporary $1210
    //
    clock();
    verifyRead(0x1210, m_memory.readRAM(0x1210));

    clock();
    verifyRead(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, expected.memory);

    QCOMPARE(m_memory.readRAM(0x1310), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    //
    // Still exactly 7 cycles. RMW abs,X does not gain an
    // additional cycle for page crossing.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedDCP::testAbsoluteY()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xDB);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xDB);

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

    QCOMPARE(m_memory.readRAM(0x1230), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedDCP::testAbsoluteYPageCross()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xDB);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1310, initialMemory);

    clock();
    verifyRead(0x1000, 0xDB);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1210, m_memory.readRAM(0x1210));

    clock();
    verifyRead(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, expected.memory);

    QCOMPARE(m_memory.readRAM(0x1310), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedDCP::testIndirectX()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xC3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20,X) -> pointer at $30/$31 -> $1234
    //
    m_memory.writeRAM(0x0030, 0x34);
    m_memory.writeRAM(0x0031, 0x12);
    m_memory.writeRAM(0x1234, initialMemory);

    clock();
    verifyRead(0x1000, 0xC3);

    clock();
    verifyRead(0x1001, 0x20);

    //
    // Dummy read from unindexed zero-page operand.
    //
    clock();
    verifyRead(0x0020, m_memory.readRAM(0x0020));

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

    QCOMPARE(m_memory.readRAM(0x1234), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    //
    // DCP (zp,X) = exactly 8 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedDCP::testIndirectXZeroPageWrap()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xC3);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F8 + $10 wraps to $08.
    // Pointer high-byte wraps inside zero page as well.
    //
    m_memory.writeRAM(0x0008, 0x34);
    m_memory.writeRAM(0x0009, 0x12);
    m_memory.writeRAM(0x1234, initialMemory);

    clock();
    verifyRead(0x1000, 0xC3);

    clock();
    verifyRead(0x1001, 0xF8);

    clock();
    verifyRead(0x00F8, m_memory.readRAM(0x00F8));

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

    QCOMPARE(m_memory.readRAM(0x1234), expected.memory);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedDCP::testIndirectY()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xD3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20),Y -> $1220 + $10 = $1230
    //
    m_memory.writeRAM(0x0020, 0x20);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xD3);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x20);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // Indexed RMW performs the dummy indexed read even without
    // a page crossing.
    //
    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyRead(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, initialMemory);

    clock();
    verifyWrite(0x1230, expected.memory);

    QCOMPARE(m_memory.readRAM(0x1230), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    //
    // DCP (zp),Y = exactly 8 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedDCP::testIndirectYPageCross()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;
    const quint8 initialStatus = 0x20;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    const quint8 finalStatus =
        expectedStatus(initialStatus, expected);

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xD3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20),Y -> $12F0 + $20 = $1310
    //
    m_memory.writeRAM(0x0020, 0xF0);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1310, initialMemory);

    clock();
    verifyRead(0x1000, 0xD3);

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
    verifyRead(0x1210, m_memory.readRAM(0x1210));

    clock();
    verifyRead(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, initialMemory);

    clock();
    verifyWrite(0x1310, expected.memory);

    QCOMPARE(m_memory.readRAM(0x1310), expected.memory);
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), finalStatus);

    //
    // Page crossing still does not add another cycle.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedDCP::testIndirectYZeroPageWrap()
{
    setupCpu();

    const quint8 accumulator = 0x40;
    const quint8 initialMemory = 0x41;

    const DCPResult expected =
        referenceDCP(accumulator, initialMemory);

    //
    // Pointer high byte after zero-page wrap is read from $0000,
    // which is the MOS6510 data-direction register.
    //
    setDataDirectionRegister(0x12);

    //
    // setDataDirectionRegister() changes A and PC.
    // Restore the state required by this test.
    //
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xD3);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer low byte at $FF and high byte at $00.
    //
    m_memory.writeRAM(0x00FF, 0x20);
    m_memory.writeRAM(0x1230, initialMemory);

    clock();
    verifyRead(0x1000, 0xD3);

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

    QCOMPARE(m_memory.readRAM(0x1230), expected.memory);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedDCP::testExhaustive()
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
            const quint8 initialAccumulator =
                static_cast<quint8>(accumulator);

            const quint8 initialMemory =
                static_cast<quint8>(memory);

            const DCPResult expected =
                referenceDCP(
                    initialAccumulator,
                    initialMemory);

            setupCpu();

            //
            // C/Z/N deliberately start set.
            // D/V must survive DCP.
            //
            const quint8 initialStatus =
                static_cast<quint8>(
                    static_cast<quint8>(
                        MOS6510StatusFlag::Unused)
                    | static_cast<quint8>(
                        MOS6510StatusFlag::Carry)
                    | static_cast<quint8>(
                        MOS6510StatusFlag::Zero)
                    | static_cast<quint8>(
                        MOS6510StatusFlag::Decimal)
                    | static_cast<quint8>(
                        MOS6510StatusFlag::Overflow)
                    | static_cast<quint8>(
                        MOS6510StatusFlag::Negative));

            const quint8 finalStatus =
                expectedStatus(
                    initialStatus,
                    expected);

            m_cpu.setAccumulator(initialAccumulator);
            m_cpu.setStatus(initialStatus);
            m_cpu.setProgramCounter(0x1000);

            m_memory.writeRAM(0x1000, 0xC7);
            m_memory.writeRAM(0x1001, 0x80);
            m_memory.writeRAM(0x0080, initialMemory);

            //
            // DCP zp:
            //
            // C1 opcode
            // C2 address
            // C3 read
            // C4 dummy write
            // C5 final write
            //
            clock();
            clock();
            clock();
            clock();
            clock();

            if (m_memory.readRAM(0x0080) != expected.memory
                || m_cpu.accumulator() != initialAccumulator
                || m_cpu.status() != finalStatus)
            {
                QFAIL(
                    qPrintable(
                        QStringLiteral(
                            "DCP mismatch: "
                            "A=$%1 M=$%2 "
                            "| actual A=$%3 M=$%4 P=$%5 "
                            "| expected A=$%6 M=$%7 P=$%8")
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
                                initialAccumulator,
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

    QCOMPARE(
        testCount,
        quint32(256 * 256));
}
