#include "MOS6510TestUndocumentedANC.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedANC::MOS6510TestUndocumentedANC()
{
}

MOS6510TestUndocumentedANC::~MOS6510TestUndocumentedANC()
{
}


MOS6510TestUndocumentedANC::ANCResult
MOS6510TestUndocumentedANC::referenceANC(
    const quint8 accumulator,
    const quint8 operand)
{
    ANCResult result;

    result.accumulator =
        static_cast<quint8>(accumulator & operand);

    result.zero =
        result.accumulator == 0;

    result.negative =
        (result.accumulator & 0x80) != 0;

    //
    // ANC copies bit 7 of the result into Carry.
    //
    result.carry =
        result.negative;

    return result;
}


quint8 MOS6510TestUndocumentedANC::expectedStatus(
    const quint8 initialStatus,
    const ANCResult& result)
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


void MOS6510TestUndocumentedANC::testImmediate_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("accumulator");
    QTest::addColumn<int>("operand");
    QTest::addColumn<int>("initialStatus");

    //
    // Test the basic behaviour with opcode $0B.
    //
    QTest::newRow("0B zero")
        << 0x0B
        << 0x00
        << 0xFF
        << 0x20;

    QTest::newRow("0B and produces zero")
        << 0x0B
        << 0xF0
        << 0x0F
        << 0x20;

    QTest::newRow("0B positive")
        << 0x0B
        << 0x7F
        << 0xFF
        << 0x20;

    QTest::newRow("0B negative and carry")
        << 0x0B
        << 0x80
        << 0xFF
        << 0x20;

    QTest::newRow("0B and mask")
        << 0x0B
        << 0xF5
        << 0x3C
        << 0x20;

    //
    // Old Carry must have no influence.
    //
    QTest::newRow("0B old carry clear")
        << 0x0B
        << 0x40
        << 0xFF
        << 0x20;

    QTest::newRow("0B old carry set")
        << 0x0B
        << 0x40
        << 0xFF
        << 0x21;

    //
    // N and C must always contain the same value.
    //
    QTest::newRow("0B NC clear")
        << 0x0B
        << 0x7F
        << 0xFF
        << 0xA1;

    QTest::newRow("0B NC set")
        << 0x0B
        << 0xFF
        << 0x80
        << 0x20;

    //
    // V is not affected.
    //
    QTest::newRow("0B overflow clear preserved")
        << 0x0B
        << 0x55
        << 0xFF
        << 0x20;

    QTest::newRow("0B overflow set preserved")
        << 0x0B
        << 0x55
        << 0xFF
        << 0x60;

    //
    // Decimal mode has no special ANC behaviour.
    // D must remain unchanged.
    //
    QTest::newRow("0B decimal clear preserved")
        << 0x0B
        << 0xAA
        << 0xFF
        << 0x20;

    QTest::newRow("0B decimal set preserved")
        << 0x0B
        << 0xAA
        << 0xFF
        << 0x28;

    //
    // Preserve unrelated status bits.
    //
    QTest::newRow("0B unrelated flags preserved")
        << 0x0B
        << 0x55
        << 0xFF
        << 0x7C;

    //
    // $2B is the second encoding of ANC.
    // Repeat the important semantic cases explicitly so that
    // this opcode is not covered only by the exhaustive test.
    //
    QTest::newRow("2B zero")
        << 0x2B
        << 0x00
        << 0xFF
        << 0x20;

    QTest::newRow("2B positive")
        << 0x2B
        << 0x7F
        << 0xFF
        << 0x20;

    QTest::newRow("2B negative and carry")
        << 0x2B
        << 0x80
        << 0xFF
        << 0x20;

    QTest::newRow("2B and mask")
        << 0x2B
        << 0xF5
        << 0x3C
        << 0x20;

    QTest::newRow("2B old carry set")
        << 0x2B
        << 0x40
        << 0xFF
        << 0x21;

    QTest::newRow("2B overflow preserved")
        << 0x2B
        << 0x55
        << 0xFF
        << 0x60;

    QTest::newRow("2B decimal preserved")
        << 0x2B
        << 0xAA
        << 0xFF
        << 0x28;
}


void MOS6510TestUndocumentedANC::testImmediate()
{
    QFETCH(int, opcode);
    QFETCH(int, accumulator);
    QFETCH(int, operand);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 instructionOpcode =
        static_cast<quint8>(opcode);

    const quint8 initialAccumulator =
        static_cast<quint8>(accumulator);

    const quint8 immediateOperand =
        static_cast<quint8>(operand);

    const quint8 status =
        static_cast<quint8>(initialStatus);

    const ANCResult expected =
        referenceANC(
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

    m_memory.writeRAM(
        0x1000,
        instructionOpcode);

    m_memory.writeRAM(
        0x1001,
        immediateOperand);

    m_memory.writeRAM(
        0x1002,
        0xEA);

    //
    // C1
    // Fetch ANC #imm opcode.
    //
    clock();

    verifyRead(
        0x1000,
        instructionOpcode);

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
    // Read immediate operand and execute ANC.
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
    // This proves that ANC itself consumes exactly two cycles.
    //
    clock();

    verifyRead(
        0x1002,
        0xEA);

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


void MOS6510TestUndocumentedANC::testExhaustive()
{
    const quint8 opcodes[] =
        {
            0x0B,
            0x2B
        };

    quint32 testCount = 0;

    for (quint8 opcodeIndex = 0;
         opcodeIndex < 2;
         ++opcodeIndex)
    {
        const quint8 opcode =
            opcodes[opcodeIndex];

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

                const ANCResult expected =
                    referenceANC(
                        initialAccumulator,
                        immediateOperand);

                setupCpu();

                //
                // C, Z and N are deliberately set so ANC must
                // calculate all affected flags again.
                //
                // D and V are deliberately set and must survive.
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

                m_cpu.setAccumulator(
                    initialAccumulator);

                m_cpu.setStatus(
                    initialStatus);

                m_cpu.setProgramCounter(
                    0x1000);

                m_memory.writeRAM(
                    0x1000,
                    opcode);

                m_memory.writeRAM(
                    0x1001,
                    immediateOperand);

                //
                // C1: opcode fetch
                // C2: immediate operand + ANC
                //
                clock();
                clock();

                if (m_cpu.accumulator()
                        != expected.accumulator
                    || m_cpu.status()
                           != finalStatus)
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "ANC mismatch: "
                                "opcode=$%1 A=$%2 operand=$%3 "
                                "| actual A=$%4 P=$%5 "
                                "| expected A=$%6 P=$%7")
                                .arg(
                                    opcode,
                                    2,
                                    16,
                                    QLatin1Char('0'))
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
    }

    QCOMPARE(
        testCount,
        quint32(256 * 256 * 2));
}
