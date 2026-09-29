#include "MOS6510TestUndocumentedRRA.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedRRA::MOS6510TestUndocumentedRRA()
{
}

MOS6510TestUndocumentedRRA::~MOS6510TestUndocumentedRRA()
{
}


MOS6510TestUndocumentedRRA::RRAResult
MOS6510TestUndocumentedRRA::referenceRRA(
    const quint8 accumulator,
    const quint8 memory,
    const bool carry,
    const bool decimal)
{
    RRAResult result;

    //
    // First part of RRA:
    //
    //     ROR memory
    //
    // The old CPU carry is shifted into bit 7.
    // Bit 0 of the old memory value becomes the carry
    // input for the following ADC.
    //
    const bool adcCarry =
        (memory & 0x01) != 0;

    result.memory =
        static_cast<quint8>(
            (memory >> 1)
            | (carry ? 0x80 : 0x00));

    const quint16 binarySum =
        static_cast<quint16>(
            accumulator)
        + static_cast<quint16>(
            result.memory)
        + static_cast<quint16>(
            adcCarry ? 1 : 0);

    const quint8 binaryResult =
        static_cast<quint8>(
            binarySum);

    result.overflow =
        ((~(accumulator ^ result.memory)
          & (accumulator ^ binaryResult)
          & 0x80) != 0);

    if (!decimal)
    {
        result.accumulator =
            binaryResult;

        result.carry =
            binarySum > 0xFF;

        result.zero =
            result.accumulator == 0;

        result.negative =
            (result.accumulator & 0x80) != 0;

        return result;
    }

    //
    // NMOS 6502 decimal ADC.
    //
    // This deliberately mirrors the independently verified
    // ADC decimal reference from MOS6510TestDecimal.
    //
    // Z comes from the unadjusted binary result.
    // N and V come from the intermediate result after
    // low-digit correction but before high-digit correction.
    //
    unsigned temp =
        (accumulator & 0x0F)
        + (result.memory & 0x0F)
        + (adcCarry ? 1U : 0U);

    if (temp > 9)
    {
        temp += 6;
    }

    if (temp <= 0x0F)
    {
        temp =
            (temp & 0x0F)
            + (accumulator & 0xF0)
            + (result.memory & 0xF0);
    }
    else
    {
        temp =
            (temp & 0x0F)
            + (accumulator & 0xF0)
            + (result.memory & 0xF0)
            + 0x10;
    }

    //
    // Z uses the unadjusted binary result.
    //
    result.zero =
        binaryResult == 0;

    //
    // N and V use the intermediate result.
    //
    result.negative =
        (temp & 0x80) != 0;

    result.overflow =
        (((accumulator ^ temp) & 0x80) != 0)
        && (((accumulator ^ result.memory) & 0x80) == 0);

    //
    // High-digit decimal correction.
    //
    if ((temp & 0x1F0) > 0x90)
    {
        temp += 0x60;
    }

    result.carry =
        (temp & 0xFF0) > 0xF0;

    result.accumulator =
        static_cast<quint8>(temp);

    return result;
}


quint8 MOS6510TestUndocumentedRRA::expectedStatus(
    const quint8 initialStatus,
    const RRAResult& result)
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


void MOS6510TestUndocumentedRRA::testZeroPage_data()
{
    QTest::addColumn<int>("accumulator");
    QTest::addColumn<int>("memory");
    QTest::addColumn<bool>("carry");
    QTest::addColumn<bool>("decimal");
    QTest::addColumn<int>("initialStatus");

    QTest::newRow("binary zero")
        << 0x00 << 0x00 << false << false << 0x20;

    QTest::newRow("ROR carry input")
        << 0x00 << 0x00 << true << false << 0x20;

    QTest::newRow("ROR carry output to ADC")
        << 0x00 << 0x01 << false << false << 0x20;

    QTest::newRow("ROR input and output carry")
        << 0x10 << 0x01 << true << false << 0x20;

    QTest::newRow("ADC carry output")
        << 0xF0 << 0x20 << false << false << 0x20;

    QTest::newRow("ADC overflow positive")
        << 0x40 << 0x40 << false << false << 0x20;

    QTest::newRow("negative result")
        << 0x80 << 0x00 << false << false << 0x20;

    QTest::newRow("old flags cleared")
        << 0x01 << 0x02 << false << false << 0xE3;

    QTest::newRow("binary unrelated flags preserved")
        << 0x11 << 0x22 << true << false << 0x3D;

    QTest::newRow("decimal simple")
        << 0x15 << 0x24 << false << true << 0x28;

    QTest::newRow("decimal carry")
        << 0x90 << 0x20 << false << true << 0x28;

    QTest::newRow("decimal ROR carry to ADC")
        << 0x09 << 0x01 << false << true << 0x28;

    QTest::newRow("decimal old carry into ROR")
        << 0x10 << 0x00 << true << true << 0x28;

    QTest::newRow("decimal unrelated flags preserved")
        << 0x25 << 0x42 << true << true << 0x3C;
}


void MOS6510TestUndocumentedRRA::testZeroPage()
{
    QFETCH(int, accumulator);
    QFETCH(int, memory);
    QFETCH(bool, carry);
    QFETCH(bool, decimal);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 initialAccumulator =
        static_cast<quint8>(
            accumulator);

    const quint8 oldMemory =
        static_cast<quint8>(
            memory);

    quint8 status =
        static_cast<quint8>(
            initialStatus);

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

    if (decimal)
    {
        status |=
            static_cast<quint8>(
                MOS6510StatusFlag::Decimal);
    }
    else
    {
        status &=
            static_cast<quint8>(
                ~static_cast<quint8>(
                    MOS6510StatusFlag::Decimal));
    }

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            carry,
            decimal);

    const quint8 finalStatus =
        expectedStatus(
            status,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x33);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x67);
    m_memory.writeRAM(0x1001, 0x80);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(
        0x0080,
        oldMemory);

    //
    // C1: Opcode fetch
    //
    clock();
    verifyRead(0x1000, 0x67);

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
    // C4: NMOS RMW dummy write
    //
    clock();
    verifyWrite(0x0080, oldMemory);

    QCOMPARE(
        m_memory.readRAM(0x0080),
        oldMemory);

    //
    // C5: Write rotated memory and execute ADC
    //
    clock();
    verifyWrite(
        0x0080,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testZeroPageX()
{
    setupCpu();

    const quint8 initialAccumulator = 0x42;
    const quint8 oldMemory = 0x81;
    const bool carry = true;
    const bool decimal = false;
    const quint8 initialStatus = 0x21;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            carry,
            decimal);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x44);
    m_cpu.setStackPointer(0x55);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x77);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x5A);
    m_memory.writeRAM(0x0030, oldMemory);

    clock();
    verifyRead(0x1000, 0x77);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x5A);

    clock();
    verifyRead(0x0030, oldMemory);

    clock();
    verifyWrite(0x0030, oldMemory);

    clock();
    verifyWrite(
        0x0030,
        expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x0030),
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


void MOS6510TestUndocumentedRRA::testZeroPageXWrap()
{
    setupCpu();

    const quint8 initialAccumulator = 0x20;
    const quint8 oldMemory = 0x03;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            false,
            false);

    const quint8 finalStatus =
        expectedStatus(
            0x20,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x77);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x00F8, 0x5A);
    m_memory.writeRAM(0x0008, oldMemory);

    clock();
    verifyRead(0x1000, 0x77);

    clock();
    verifyRead(0x1001, 0xF8);

    clock();
    verifyRead(0x00F8, 0x5A);

    clock();
    verifyRead(0x0008, oldMemory);

    clock();
    verifyWrite(0x0008, oldMemory);

    clock();
    verifyWrite(
        0x0008,
        expected.memory);

    QCOMPARE(
        m_memory.readRAM(0x0008),
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


void MOS6510TestUndocumentedRRA::testAbsolute()
{
    setupCpu();

    const quint8 initialAccumulator = 0x45;
    const quint8 oldMemory = 0x83;
    const quint8 initialStatus = 0x21;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            true,
            false);

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

    m_memory.writeRAM(0x1000, 0x6F);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, oldMemory);

    clock();
    verifyRead(0x1000, 0x6F);

    clock();
    verifyRead(0x1001, 0x34);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, oldMemory);

    clock();
    verifyWrite(
        0x1234,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testAbsoluteX()
{
    setupCpu();

    const quint8 initialAccumulator = 0x42;
    const quint8 oldMemory = 0x81;
    const quint8 initialStatus = 0x21;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x7F);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x7F);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Fixed indexed-RMW dummy read.
    //
    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(
        0x1230,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testAbsoluteXPageCross()
{
    setupCpu();

    const quint8 initialAccumulator = 0x45;
    const quint8 oldMemory = 0x83;
    const quint8 initialStatus = 0x20;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            false,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x7F);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, oldMemory);

    clock();
    verifyRead(0x1000, 0x7F);

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
    verifyWrite(
        0x1310,
        expected.memory);

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
    // Page crossing must not add a cycle.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedRRA::testAbsoluteY()
{
    setupCpu();

    const quint8 initialAccumulator = 0x42;
    const quint8 oldMemory = 0x81;
    const quint8 initialStatus = 0x21;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x7B);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x7B);

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
    verifyWrite(
        0x1230,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testAbsoluteYPageCross()
{
    setupCpu();

    const quint8 initialAccumulator = 0x45;
    const quint8 oldMemory = 0x83;
    const quint8 initialStatus = 0x20;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            false,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x7B);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, oldMemory);

    clock();
    verifyRead(0x1000, 0x7B);

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
    verifyWrite(
        0x1310,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testIndirectX()
{
    setupCpu();

    const quint8 initialAccumulator = 0x42;
    const quint8 oldMemory = 0x81;
    const quint8 initialStatus = 0x21;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x63);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x5A);
    m_memory.writeRAM(0x0030, 0x34);
    m_memory.writeRAM(0x0031, 0x12);
    m_memory.writeRAM(0x1234, oldMemory);

    clock();
    verifyRead(0x1000, 0x63);

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
    verifyWrite(
        0x1234,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testIndirectXZeroPageWrap()
{
    setupCpu();

    const quint8 initialAccumulator = 0x20;
    const quint8 oldMemory = 0x03;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            false,
            false);

    const quint8 finalStatus =
        expectedStatus(
            0x20,
            expected);

    //
    // Pointer high byte after zero-page wrap is read from $0000,
    // which is the MOS6510 data-direction register.
    //
    setDataDirectionRegister(0x12);

    //
    // Restore the complete CPU state required by this test.
    //
    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setXRegister(0x01);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x63);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $FE + X = $FF.
    // High pointer byte must wrap to $00.
    //
    m_memory.writeRAM(0x00FE, 0x5A);
    m_memory.writeRAM(0x00FF, 0x34);

    m_memory.writeRAM(0x1234, oldMemory);

    clock();
    verifyRead(0x1000, 0x63);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x5A);

    clock();
    verifyRead(0x00FF, 0x34);

    clock();
    verifyReadCycle(0x0000);

    clock();
    verifyRead(0x1234, oldMemory);

    clock();
    verifyWrite(0x1234, oldMemory);

    clock();
    verifyWrite(
        0x1234,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testIndirectY()
{
    setupCpu();

    const quint8 initialAccumulator = 0x42;
    const quint8 oldMemory = 0x81;
    const quint8 initialStatus = 0x21;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            true,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x73);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x20);
    m_memory.writeRAM(0x0021, 0x12);

    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x73);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x20);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // Fixed indexed-RMW dummy read.
    //
    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(
        0x1230,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testIndirectYPageCross()
{
    setupCpu();

    const quint8 initialAccumulator = 0x45;
    const quint8 oldMemory = 0x83;
    const quint8 initialStatus = 0x20;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            false,
            false);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setYRegister(0x20);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x73);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer = $12F0
    // + Y $20 = $1310
    // intermediate address = $1210
    //
    m_memory.writeRAM(0x0020, 0xF0);
    m_memory.writeRAM(0x0021, 0x12);

    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, oldMemory);

    clock();
    verifyRead(0x1000, 0x73);

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
    verifyWrite(
        0x1310,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testIndirectYZeroPageWrap()
{
    setupCpu();

    const quint8 initialAccumulator = 0x20;
    const quint8 oldMemory = 0x03;

    const RRAResult expected =
        referenceRRA(
            initialAccumulator,
            oldMemory,
            false,
            false);

    const quint8 finalStatus =
        expectedStatus(
            0x20,
            expected);

    //
    // Pointer high byte after zero-page wrap is read from $0000,
    // which is the MOS6510 data-direction register.
    //
    setDataDirectionRegister(0x12);

    //
    // Restore the complete CPU state required by this test.
    //
    m_cpu.setAccumulator(
        initialAccumulator);

    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x73);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer low at $FF, high at $00.
    // Base = $1220, +Y = $1230.
    //
    m_memory.writeRAM(0x00FF, 0x20);

    m_memory.writeRAM(0x1230, oldMemory);

    clock();
    verifyRead(0x1000, 0x73);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x20);

    clock();
    verifyReadCycle(0x0000);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyRead(0x1230, oldMemory);

    clock();
    verifyWrite(0x1230, oldMemory);

    clock();
    verifyWrite(
        0x1230,
        expected.memory);

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


void MOS6510TestUndocumentedRRA::testExhaustiveBinary()
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
                // D is clear. I, B and U are set so
                // their preservation is checked.
                // C is inserted below.
                //
                quint8 initialStatus = 0x34;

                if (carry != 0)
                {
                    initialStatus |=
                        static_cast<quint8>(
                            MOS6510StatusFlag::Carry);
                }

                const RRAResult expected =
                    referenceRRA(
                        initialAccumulator,
                        oldMemory,
                        carry != 0,
                        false);

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
                    0x67);

                m_memory.writeRAM(
                    0x1001,
                    0x80);

                m_memory.writeRAM(
                    0x0080,
                    oldMemory);

                //
                // RRA zp = exactly 5 cycles.
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
                                "RRA binary mismatch: "
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


void MOS6510TestUndocumentedRRA::testExhaustiveDecimal()
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
                // Decimal set, plus I/B/U to verify
                // preservation.
                //
                quint8 initialStatus = 0x3C;

                if (carry != 0)
                {
                    initialStatus |=
                        static_cast<quint8>(
                            MOS6510StatusFlag::Carry);
                }

                const RRAResult expected =
                    referenceRRA(
                        initialAccumulator,
                        oldMemory,
                        carry != 0,
                        true);

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
                    0x67);

                m_memory.writeRAM(
                    0x1001,
                    0x80);

                m_memory.writeRAM(
                    0x0080,
                    oldMemory);

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
                                "RRA decimal mismatch: "
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
