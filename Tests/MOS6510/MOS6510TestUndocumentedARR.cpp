#include "MOS6510TestUndocumentedARR.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedARR::ARRResult
MOS6510TestUndocumentedARR::referenceARR(
    const quint8 accumulator,
    const quint8 operand,
    const bool carryIn,
    const bool decimal)
{
    ARRResult result;

    const quint8 andResult =
        static_cast<quint8>(accumulator & operand);

    quint8 rotated =
        static_cast<quint8>(
            (andResult >> 1)
            | (carryIn ? 0x80 : 0x00));

    //
    // N and Z are determined from the result of the ROR before
    // any decimal correction.
    //
    result.negative = (rotated & 0x80) != 0;
    result.zero = rotated == 0;

    if (!decimal)
    {
        //
        // Binary mode:
        //
        // C = bit 6 of the rotated result
        // V = bit 6 XOR bit 5 of the rotated result
        //
        result.carry = (rotated & 0x40) != 0;
        result.overflow =
            (((rotated >> 6) ^ (rotated >> 5)) & 0x01) != 0;

        result.accumulator = rotated;

        return result;
    }

    //
    // Decimal mode.
    //
    // V is determined before decimal correction.
    //
    result.overflow =
        ((rotated ^ andResult) & 0x40) != 0;

    const quint8 lowNibble =
        static_cast<quint8>(andResult & 0x0F);

    const quint8 highNibble =
        static_cast<quint8>(andResult >> 4);

    //
    // Decimal correction of the low nibble.
    //
    if (static_cast<quint8>(
            lowNibble + (lowNibble & 0x01)) > 5)
    {
        rotated =
            static_cast<quint8>(
                (rotated & 0xF0)
                | ((rotated + 0x06) & 0x0F));
    }

    //
    // Decimal correction of the high nibble.
    //
    result.carry =
        static_cast<quint8>(
            highNibble + (highNibble & 0x01)) > 5;

    if (result.carry)
    {
        rotated =
            static_cast<quint8>(rotated + 0x60);
    }

    result.accumulator = rotated;

    return result;
}


quint8 MOS6510TestUndocumentedARR::expectedStatus(
    const quint8 initialStatus,
    const ARRResult& result)
{
    const quint8 affectedFlags =
        static_cast<quint8>(
            static_cast<quint8>(MOS6510StatusFlag::Carry)
            | static_cast<quint8>(MOS6510StatusFlag::Zero)
            | static_cast<quint8>(MOS6510StatusFlag::Overflow)
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

    if (result.overflow)
    {
        status |=
            static_cast<quint8>(MOS6510StatusFlag::Overflow);
    }

    if (result.negative)
    {
        status |=
            static_cast<quint8>(MOS6510StatusFlag::Negative);
    }

    return status;
}

// ============================================================================================
// Construction
// ============================================================================================

MOS6510TestUndocumentedARR::MOS6510TestUndocumentedARR()
{
}

MOS6510TestUndocumentedARR::~MOS6510TestUndocumentedARR()
{
}


// ============================================================================================
// ARR #Immediate - Binary
// ============================================================================================

void MOS6510TestUndocumentedARR::testImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");

    //
    // Basic AND and ROR behaviour.
    //
    QTest::newRow("zero")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("and mask")
        << quint8(0xF0)
        << quint8(0x3C)
        << quint8(0);

    QTest::newRow("carry in")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(1);

    QTest::newRow("all bits carry clear")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("all bits carry set")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(1);

    //
    // Explicitly cover all four combinations of result bits 6 and 5.
    //
    // bit6=0 bit5=0 -> C=0 V=0
    //
    QTest::newRow("CV 00")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0);

    //
    // bit6=0 bit5=1 -> C=0 V=1
    //
    // AND result $40 -> rotated $20.
    //
    QTest::newRow("CV 01")
        << quint8(0x40)
        << quint8(0xFF)
        << quint8(0);

    //
    // bit6=1 bit5=0 -> C=1 V=1
    //
    // AND result $80 -> rotated $40.
    //
    QTest::newRow("CV 10")
        << quint8(0x80)
        << quint8(0xFF)
        << quint8(0);

    //
    // bit6=1 bit5=1 -> C=1 V=0
    //
    // AND result $C0 -> rotated $60.
    //
    QTest::newRow("CV 11")
        << quint8(0xC0)
        << quint8(0xFF)
        << quint8(0);
}

void MOS6510TestUndocumentedARR::testImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);

    setupCpu();

    quint8 initialStatus = 0x34;

    if (carryIn != 0)
    {
        initialStatus |=
            static_cast<quint8>(MOS6510StatusFlag::Carry);
    }
    else
    {
        initialStatus &=
            static_cast<quint8>(
                ~static_cast<quint8>(
                    MOS6510StatusFlag::Carry));
    }

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x6B);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);

    const ARRResult expected =
        referenceARR(
            accumulator,
            operand,
            carryIn != 0,
            false);

    const quint8 status =
        expectedStatus(initialStatus, expected);

    // Cycle 1: Opcode fetch
    clock();
    verifyRead(0x1000, 0x6B);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: Read immediate operand and execute ARR
    clock();
    verifyRead(0x1001, operand);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expected.accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), status);

    // Cycle 3: Fetch next opcode
    //
    // This proves that ARR takes exactly two cycles.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expected.accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), status);
}


// ============================================================================================
// ARR #Immediate - Decimal
// ============================================================================================

void MOS6510TestUndocumentedARR::testImmediateDecimal_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");

    //
    // No decimal correction.
    //
    QTest::newRow("no correction")
        << quint8(0x24)
        << quint8(0xFF)
        << quint8(0);

    //
    // Low nibble correction only.
    //
    QTest::newRow("low correction")
        << quint8(0x0F)
        << quint8(0xFF)
        << quint8(0);

    //
    // High nibble correction only.
    //
    QTest::newRow("high correction")
        << quint8(0xF0)
        << quint8(0xFF)
        << quint8(0);

    //
    // Both decimal corrections.
    //
    QTest::newRow("both corrections")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0);

    //
    // Carry input becomes bit 7 before decimal correction.
    //
    QTest::newRow("carry in")
        << quint8(0x24)
        << quint8(0xFF)
        << quint8(1);

    //
    // AND masking must happen before all ARR processing.
    //
    QTest::newRow("and mask")
        << quint8(0xFF)
        << quint8(0x69)
        << quint8(0);

    //
    // Invalid BCD values are intentional. NMOS ARR has defined
    // hardware behaviour for them and they are important test cases.
    //
    QTest::newRow("invalid low BCD")
        << quint8(0x0A)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("invalid high BCD")
        << quint8(0xA0)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("invalid both BCD")
        << quint8(0xAA)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("invalid both BCD carry")
        << quint8(0xAA)
        << quint8(0xFF)
        << quint8(1);

    //
    // Additional values around decimal correction boundaries.
    //
    QTest::newRow("low boundary 5")
        << quint8(0x05)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("low boundary 6")
        << quint8(0x06)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("high boundary 5")
        << quint8(0x50)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("high boundary 6")
        << quint8(0x60)
        << quint8(0xFF)
        << quint8(0);

    //
    // Values useful for checking V calculation.
    //
    QTest::newRow("overflow clear")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0);

    QTest::newRow("overflow set")
        << quint8(0x40)
        << quint8(0xFF)
        << quint8(0);
}

void MOS6510TestUndocumentedARR::testImmediateDecimal()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);

    setupCpu();

    quint8 initialStatus =
        static_cast<quint8>(
            0x34
            | static_cast<quint8>(
                MOS6510StatusFlag::Decimal));

    if (carryIn != 0)
    {
        initialStatus |=
            static_cast<quint8>(MOS6510StatusFlag::Carry);
    }
    else
    {
        initialStatus &=
            static_cast<quint8>(
                ~static_cast<quint8>(
                    MOS6510StatusFlag::Carry));
    }

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x6B);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);

    const ARRResult expected =
        referenceARR(
            accumulator,
            operand,
            carryIn != 0,
            true);

    const quint8 status =
        expectedStatus(initialStatus, expected);

    // Cycle 1: Opcode fetch
    clock();
    verifyRead(0x1000, 0x6B);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: Read immediate operand and execute decimal ARR
    clock();
    verifyRead(0x1001, operand);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expected.accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), status);

    // Cycle 3: Fetch next opcode
    //
    // Decimal mode does not change the instruction timing.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expected.accumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), status);
}


// ============================================================================================
// Exhaustive ARR verification
// ============================================================================================

void MOS6510TestUndocumentedARR::testExhaustive()
{
    quint32 testCount = 0;

    for (int decimal = 0; decimal <= 1; ++decimal)
    {
        for (int accumulator = 0;
             accumulator <= 0xFF;
             ++accumulator)
        {
            for (int operand = 0;
                 operand <= 0xFF;
                 ++operand)
            {
                for (int carryIn = 0;
                     carryIn <= 1;
                     ++carryIn)
                {
                    ++testCount;

                    const ARRResult expected =
                        referenceARR(
                            static_cast<quint8>(accumulator),
                            static_cast<quint8>(operand),
                            carryIn != 0,
                            decimal != 0);

                    setupCpu();

                    quint8 initialStatus = 0x34;

                    if (decimal != 0)
                    {
                        initialStatus |=
                            static_cast<quint8>(
                                MOS6510StatusFlag::Decimal);
                    }
                    else
                    {
                        initialStatus &=
                            static_cast<quint8>(
                                ~static_cast<quint8>(
                                    MOS6510StatusFlag::Decimal));
                    }

                    if (carryIn != 0)
                    {
                        initialStatus |=
                            static_cast<quint8>(
                                MOS6510StatusFlag::Carry);
                    }
                    else
                    {
                        initialStatus &=
                            static_cast<quint8>(
                                ~static_cast<quint8>(
                                    MOS6510StatusFlag::Carry));
                    }

                    m_cpu.setAccumulator(
                        static_cast<quint8>(accumulator));

                    m_cpu.setStatus(initialStatus);

                    m_memory.writeRAM(0x1000, 0x6B);
                    m_memory.writeRAM(
                        0x1001,
                        static_cast<quint8>(operand));

                    m_cpu.setProgramCounter(0x1000);

                    //
                    // ARR #imm = exactly two clocks.
                    //
                    m_cpu.clock();
                    m_cpu.clock();

                    const quint8 actualStatus =
                        m_cpu.status();

                    const bool actualCarry =
                        (actualStatus
                         & static_cast<quint8>(
                             MOS6510StatusFlag::Carry)) != 0;

                    const bool actualZero =
                        (actualStatus
                         & static_cast<quint8>(
                             MOS6510StatusFlag::Zero)) != 0;

                    const bool actualOverflow =
                        (actualStatus
                         & static_cast<quint8>(
                             MOS6510StatusFlag::Overflow)) != 0;

                    const bool actualNegative =
                        (actualStatus
                         & static_cast<quint8>(
                             MOS6510StatusFlag::Negative)) != 0;

                    if (m_cpu.accumulator()
                            != expected.accumulator
                        || actualCarry
                               != expected.carry
                        || actualZero
                               != expected.zero
                        || actualOverflow
                               != expected.overflow
                        || actualNegative
                               != expected.negative)
                    {
                        QFAIL(
                            qPrintable(
                                QStringLiteral(
                                    "ARR mismatch: "
                                    "A=$%1 operand=$%2 C=%3 D=%4 | "
                                    "actual A=$%5 N=%6 V=%7 Z=%8 C=%9 | "
                                    "expected A=$%10 N=%11 V=%12 Z=%13 C=%14")
                                    .arg(
                                        accumulator,
                                        2,
                                        16,
                                        QLatin1Char('0'))
                                    .arg(
                                        operand,
                                        2,
                                        16,
                                        QLatin1Char('0'))
                                    .arg(carryIn)
                                    .arg(decimal)
                                    .arg(
                                        m_cpu.accumulator(),
                                        2,
                                        16,
                                        QLatin1Char('0'))
                                    .arg(actualNegative)
                                    .arg(actualOverflow)
                                    .arg(actualZero)
                                    .arg(actualCarry)
                                    .arg(
                                        expected.accumulator,
                                        2,
                                        16,
                                        QLatin1Char('0'))
                                    .arg(expected.negative)
                                    .arg(expected.overflow)
                                    .arg(expected.zero)
                                    .arg(expected.carry)));
                    }
                }
            }
        }
    }

    QCOMPARE(
        testCount,
        quint32(256 * 256 * 2 * 2));
}
