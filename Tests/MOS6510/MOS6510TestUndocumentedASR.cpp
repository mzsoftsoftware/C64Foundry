#include "MOS6510TestUndocumentedASR.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedASR::MOS6510TestUndocumentedASR()
{
}

MOS6510TestUndocumentedASR::~MOS6510TestUndocumentedASR()
{
}


MOS6510TestUndocumentedASR::ASRResult
MOS6510TestUndocumentedASR::referenceASR(
    const quint8 accumulator,
    const quint8 operand)
{
    ASRResult result;

    const quint8 andResult =
        static_cast<quint8>(accumulator & operand);

    result.carry =
        (andResult & 0x01) != 0;

    result.accumulator =
        static_cast<quint8>(andResult >> 1);

    result.zero =
        result.accumulator == 0;

    result.negative =
        (result.accumulator & 0x80) != 0;

    return result;
}


quint8 MOS6510TestUndocumentedASR::expectedStatus(
    const quint8 initialStatus,
    const ASRResult& result)
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


void MOS6510TestUndocumentedASR::testImmediate_data()
{
    QTest::addColumn<int>("accumulator");
    QTest::addColumn<int>("operand");
    QTest::addColumn<int>("initialStatus");

    //
    // Basic result and Zero flag.
    //
    QTest::newRow("zero")
        << 0x00
        << 0xFF
        << 0x20;

    QTest::newRow("and produces zero")
        << 0xF0
        << 0x0F
        << 0x20;

    //
    // Carry comes from bit 0 of the AND result.
    //
    QTest::newRow("carry clear")
        << 0xFE
        << 0xFF
        << 0x21;

    QTest::newRow("carry set")
        << 0xFF
        << 0xFF
        << 0x20;

    //
    // Old Carry must have no influence on the result.
    //
    QTest::newRow("old carry clear")
        << 0x80
        << 0xFF
        << 0x20;

    QTest::newRow("old carry set")
        << 0x80
        << 0xFF
        << 0x21;

    //
    // AND must happen before the shift.
    //
    QTest::newRow("and mask")
        << 0xFF
        << 0x3C
        << 0x20;

    //
    // Highest possible result is $7F.
    // N must therefore always be clear.
    //
    QTest::newRow("negative cleared")
        << 0xFF
        << 0xFF
        << 0xA0;

    //
    // V is not affected.
    //
    QTest::newRow("overflow clear preserved")
        << 0xAA
        << 0xFF
        << 0x20;

    QTest::newRow("overflow set preserved")
        << 0xAA
        << 0xFF
        << 0x60;

    //
    // Decimal mode has no special ASR behaviour and D must
    // remain unchanged.
    //
    QTest::newRow("decimal clear preserved")
        << 0x69
        << 0xFF
        << 0x20;

    QTest::newRow("decimal set preserved")
        << 0x69
        << 0xFF
        << 0x28;

    //
    // Preserve unrelated status bits.
    //
    QTest::newRow("unrelated flags preserved")
        << 0x55
        << 0xFF
        << 0x7C;
}


void MOS6510TestUndocumentedASR::testImmediate()
{
    QFETCH(int, accumulator);
    QFETCH(int, operand);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 initialAccumulator =
        static_cast<quint8>(accumulator);

    const quint8 immediateOperand =
        static_cast<quint8>(operand);

    const quint8 status =
        static_cast<quint8>(initialStatus);

    const ASRResult expected =
        referenceASR(
            initialAccumulator,
            immediateOperand);

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

    m_memory.writeRAM(0x1000, 0x4B);
    m_memory.writeRAM(0x1001, immediateOperand);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // C1
    // Fetch ASR #imm opcode.
    //
    clock();

    verifyRead(0x1000, 0x4B);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    QCOMPARE(
        m_cpu.accumulator(),
        initialAccumulator);

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
        status);

    //
    // C2
    // Read immediate operand and execute ASR.
    //
    clock();

    verifyRead(
        0x1001,
        immediateOperand);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1002));

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

    //
    // C3
    // The next clock must already fetch the following opcode.
    // This proves that ASR itself consumes exactly two cycles.
    //
    clock();

    verifyRead(0x1002, 0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

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
}


void MOS6510TestUndocumentedASR::testExhaustive()
{
    quint32 testCount = 0;

    for (quint16 accumulator = 0;
         accumulator <= 0xFF;
         ++accumulator)
    {
        for (quint16 operand = 0;
             operand <= 0xFF;
             ++operand)
        {
            const quint8 initialAccumulator =
                static_cast<quint8>(accumulator);

            const quint8 immediateOperand =
                static_cast<quint8>(operand);

            const ASRResult expected =
                referenceASR(
                    initialAccumulator,
                    immediateOperand);

            setupCpu();

            //
            // Set C, N and Z deliberately so ASR has to
            // overwrite all affected flags.
            //
            // V and D are deliberately set and must survive.
            //
            const quint8 initialStatus =
                static_cast<quint8>(
                    static_cast<quint8>(MOS6510StatusFlag::Unused)
                    | static_cast<quint8>(MOS6510StatusFlag::Carry)
                    | static_cast<quint8>(MOS6510StatusFlag::Zero)
                    | static_cast<quint8>(MOS6510StatusFlag::Decimal)
                    | static_cast<quint8>(MOS6510StatusFlag::Overflow)
                    | static_cast<quint8>(MOS6510StatusFlag::Negative));

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
                0x4B);

            m_memory.writeRAM(
                0x1001,
                immediateOperand);

            //
            // C1: opcode fetch
            // C2: immediate operand + ASR
            //
            clock();
            clock();

            if (m_cpu.accumulator() != expected.accumulator
                || m_cpu.status() != finalStatus)
            {
                QFAIL(
                    qPrintable(
                        QStringLiteral(
                            "ASR mismatch: "
                            "A=$%1 operand=$%2 "
                            "| actual A=$%3 P=$%4 "
                            "| expected A=$%5 P=$%6")
                            .arg(
                                initialAccumulator,
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                immediateOperand,
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                m_cpu.accumulator(),
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
