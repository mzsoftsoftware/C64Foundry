#include "MOS6510TestUndocumentedLAS.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedLAS::MOS6510TestUndocumentedLAS()
{
}

MOS6510TestUndocumentedLAS::~MOS6510TestUndocumentedLAS()
{
}


MOS6510TestUndocumentedLAS::LASResult
MOS6510TestUndocumentedLAS::referenceLAS(
    const quint8 memory,
    const quint8 stackPointer)
{
    LASResult result;

    result.value =
        static_cast<quint8>(
            memory & stackPointer);

    result.zero =
        result.value == 0;

    result.negative =
        (result.value & 0x80) != 0;

    return result;
}


quint8 MOS6510TestUndocumentedLAS::expectedStatus(
    const quint8 initialStatus,
    const LASResult& result)
{
    const quint8 affectedFlags =
        static_cast<quint8>(
            static_cast<quint8>(
                MOS6510StatusFlag::Zero)
            | static_cast<quint8>(
                MOS6510StatusFlag::Negative));

    quint8 status =
        static_cast<quint8>(
            initialStatus & ~affectedFlags);

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


void MOS6510TestUndocumentedLAS::testAbsoluteY_data()
{
    QTest::addColumn<int>("memory");
    QTest::addColumn<int>("stackPointer");
    QTest::addColumn<int>("initialStatus");

    QTest::newRow("zero from memory")
        << 0x00 << 0xFF << 0x20;

    QTest::newRow("zero from stack")
        << 0xFF << 0x00 << 0x20;

    QTest::newRow("zero from mask")
        << 0x0F << 0xF0 << 0x20;

    QTest::newRow("positive")
        << 0x7F << 0xFF << 0x20;

    QTest::newRow("negative")
        << 0xFF << 0x80 << 0x20;

    QTest::newRow("all bits")
        << 0xFF << 0xFF << 0x20;

    QTest::newRow("mixed mask")
        << 0xA5 << 0xF0 << 0x20;

    QTest::newRow("old zero cleared")
        << 0x01 << 0x01 << 0x22;

    QTest::newRow("old negative cleared")
        << 0x01 << 0x01 << 0xA0;

    //
    // C/V/D/I/B/U must be preserved.
    //
    QTest::newRow("unrelated flags preserved")
        << 0x40 << 0xFF << 0x7D;
}


void MOS6510TestUndocumentedLAS::testAbsoluteY()
{
    QFETCH(int, memory);
    QFETCH(int, stackPointer);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 memoryValue =
        static_cast<quint8>(memory);

    const quint8 initialStackPointer =
        static_cast<quint8>(stackPointer);

    const quint8 status =
        static_cast<quint8>(initialStatus);

    const LASResult expected =
        referenceLAS(
            memoryValue,
            initialStackPointer);

    const quint8 finalStatus =
        expectedStatus(
            status,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(initialStackPointer);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    //
    // $BB = LAS/LDS Absolute,Y
    //
    m_memory.writeRAM(0x1000, 0xBB);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $1220 + Y($10) = $1230
    //
    m_memory.writeRAM(0x1230, memoryValue);

    //
    // C1: Opcode fetch
    //
    clock();
    verifyRead(0x1000, 0xBB);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    //
    // C2: Address low
    //
    clock();
    verifyRead(0x1001, 0x20);

    //
    // C3: Address high + Y indexing
    //
    clock();
    verifyRead(0x1002, 0x12);

    //
    // C4: Operand + LAS
    //
    clock();
    verifyRead(0x1230, memoryValue);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.value);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.value);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x10));

    QCOMPARE(
        m_cpu.stackPointer(),
        expected.value);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

    //
    // C5: Next opcode proves exactly 4 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1004));
}


void MOS6510TestUndocumentedLAS::testAbsoluteYPageCross()
{
    setupCpu();

    const quint8 memoryValue = 0xA5;
    const quint8 initialStackPointer = 0xF0;
    const quint8 initialStatus = 0x21;

    const LASResult expected =
        referenceLAS(
            memoryValue,
            initialStackPointer);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x20);
    m_cpu.setStackPointer(initialStackPointer);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xBB);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12F0 + Y($20) = $1310
    //
    // During page crossing the NMOS CPU first accesses
    // the intermediate address $1210.
    //
    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, memoryValue);

    //
    // C1: Opcode fetch
    //
    clock();
    verifyRead(0x1000, 0xBB);

    //
    // C2: Address low
    //
    clock();
    verifyRead(0x1001, 0xF0);

    //
    // C3: Address high + Y indexing
    //
    clock();
    verifyRead(0x1002, 0x12);

    //
    // C4: Wrong-page dummy read
    //
    clock();
    verifyRead(0x1210, 0x5A);

    //
    // Registers must not have been modified by
    // the dummy read.
    //
    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x11));

    QCOMPARE(
        m_cpu.xRegister(),
        quint8(0x22));

    QCOMPARE(
        m_cpu.stackPointer(),
        initialStackPointer);

    QCOMPARE(
        m_cpu.status(),
        initialStatus);

    //
    // C5: Actual operand + LAS
    //
    clock();
    verifyRead(0x1310, memoryValue);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.value);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.value);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x20));

    QCOMPARE(
        m_cpu.stackPointer(),
        expected.value);

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

    //
    // C6: Next opcode proves exactly 5 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1004));
}


void MOS6510TestUndocumentedLAS::testExhaustive()
{
    quint32 testCount = 0;

    for (quint16 memory = 0;
         memory <= 0xFF;
         ++memory)
    {
        for (quint16 stackPointer = 0;
             stackPointer <= 0xFF;
             ++stackPointer)
        {
            setupCpu();

            const quint8 memoryValue =
                static_cast<quint8>(
                    memory);

            const quint8 initialStackPointer =
                static_cast<quint8>(
                    stackPointer);

            //
            // All flags set deliberately.
            // LAS may only modify Z and N.
            //
            const quint8 initialStatus = 0xFF;

            const LASResult expected =
                referenceLAS(
                    memoryValue,
                    initialStackPointer);

            const quint8 finalStatus =
                expectedStatus(
                    initialStatus,
                    expected);

            m_cpu.setAccumulator(0x55);
            m_cpu.setXRegister(0xAA);
            m_cpu.setYRegister(0x10);
            m_cpu.setStackPointer(
                initialStackPointer);
            m_cpu.setStatus(
                initialStatus);
            m_cpu.setProgramCounter(
                0x1000);

            m_memory.writeRAM(
                0x1000,
                0xBB);

            m_memory.writeRAM(
                0x1001,
                0x20);

            m_memory.writeRAM(
                0x1002,
                0x12);

            m_memory.writeRAM(
                0x1230,
                memoryValue);

            //
            // LAS abs,Y without page crossing:
            // exactly 4 cycles.
            //
            clock();
            clock();
            clock();
            clock();

            if (m_cpu.accumulator()
                    != expected.value
                || m_cpu.xRegister()
                       != expected.value
                || m_cpu.stackPointer()
                       != expected.value
                || m_cpu.status()
                       != finalStatus
                || m_cpu.yRegister()
                       != 0x10)
            {
                QFAIL(
                    qPrintable(
                        QStringLiteral(
                            "LAS mismatch: "
                            "M=$%1 SP=$%2 "
                            "| actual A=$%3 X=$%4 SP=$%5 P=$%6 "
                            "| expected value=$%7 P=$%8")
                            .arg(
                                memoryValue,
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                initialStackPointer,
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                m_cpu.accumulator(),
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                m_cpu.xRegister(),
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                m_cpu.stackPointer(),
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                m_cpu.status(),
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                expected.value,
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
