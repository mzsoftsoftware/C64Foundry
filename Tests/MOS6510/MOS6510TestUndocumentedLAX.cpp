#include "MOS6510TestUndocumentedLAX.h"

#include <QtTest>

#include "C64/CPU/MOS6510StatusRegister.h"


MOS6510TestUndocumentedLAX::MOS6510TestUndocumentedLAX()
{
}

MOS6510TestUndocumentedLAX::~MOS6510TestUndocumentedLAX()
{
}


MOS6510TestUndocumentedLAX::LAXResult
MOS6510TestUndocumentedLAX::referenceLAX(
    const quint8 operand)
{
    LAXResult result;

    result.accumulator = operand;
    result.x = operand;

    result.zero =
        operand == 0;

    result.negative =
        (operand & 0x80) != 0;

    return result;
}


quint8 MOS6510TestUndocumentedLAX::expectedStatus(
    const quint8 initialStatus,
    const LAXResult& result)
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


void MOS6510TestUndocumentedLAX::testZeroPage_data()
{
    QTest::addColumn<int>("operand");
    QTest::addColumn<int>("initialStatus");

    QTest::newRow("zero")
        << 0x00 << 0x20;

    QTest::newRow("positive")
        << 0x01 << 0x20;

    QTest::newRow("positive maximum")
        << 0x7F << 0x20;

    QTest::newRow("negative minimum")
        << 0x80 << 0x20;

    QTest::newRow("negative")
        << 0xA5 << 0x20;

    QTest::newRow("negative maximum")
        << 0xFF << 0x20;

    QTest::newRow("old zero overwritten")
        << 0x01 << 0x22;

    QTest::newRow("old negative overwritten")
        << 0x01 << 0xA0;

    QTest::newRow("unrelated flags preserved")
        << 0x40 << 0x7D;
}


void MOS6510TestUndocumentedLAX::testZeroPage()
{
    QFETCH(int, operand);
    QFETCH(int, initialStatus);

    setupCpu();

    const quint8 value =
        static_cast<quint8>(operand);

    const quint8 status =
        static_cast<quint8>(initialStatus);

    const LAXResult expected =
        referenceLAX(value);

    const quint8 finalStatus =
        expectedStatus(
            status,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xA7);
    m_memory.writeRAM(0x1001, 0x80);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0080, value);

    //
    // C1: Opcode fetch
    //
    clock();
    verifyRead(0x1000, 0xA7);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    //
    // C2: Zero-page address
    //
    clock();
    verifyRead(0x1001, 0x80);

    //
    // C3: Read operand and load A/X
    //
    clock();
    verifyRead(0x0080, value);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

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
    // C4: Next opcode proves exactly 3 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));
}


void MOS6510TestUndocumentedLAX::testZeroPageY()
{
    setupCpu();

    const quint8 operand = 0xA5;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xB7);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x5A);
    m_memory.writeRAM(0x0030, operand);

    //
    // C1: Opcode
    //
    clock();
    verifyRead(0x1000, 0xB7);

    //
    // C2: Base zero-page address
    //
    clock();
    verifyRead(0x1001, 0x20);

    //
    // C3: Indexed zero-page dummy read
    //
    clock();
    verifyRead(0x0020, 0x5A);

    //
    // C4: Effective operand
    //
    clock();
    verifyRead(0x0030, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x10));

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
    // LAX zp,Y = exactly 4 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testZeroPageYWrap()
{
    setupCpu();

    const quint8 operand = 0x42;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xB7);
    m_memory.writeRAM(0x1001, 0xF8);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F8 + $10 wraps to $08.
    //
    m_memory.writeRAM(0x00F8, 0x5A);
    m_memory.writeRAM(0x0008, operand);

    clock();
    verifyRead(0x1000, 0xB7);

    clock();
    verifyRead(0x1001, 0xF8);

    clock();
    verifyRead(0x00F8, 0x5A);

    clock();
    verifyRead(0x0008, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testAbsolute()
{
    setupCpu();

    const quint8 operand = 0xA5;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xAF);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, operand);

    //
    // C1: Opcode
    //
    clock();
    verifyRead(0x1000, 0xAF);

    //
    // C2: Address low
    //
    clock();
    verifyRead(0x1001, 0x34);

    //
    // C3: Address high
    //
    clock();
    verifyRead(0x1002, 0x12);

    //
    // C4: Operand
    //
    clock();
    verifyRead(0x1234, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

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
        quint16(0x1003));

    //
    // LAX abs = exactly 4 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedLAX::testAbsoluteY()
{
    setupCpu();

    const quint8 operand = 0x42;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xBF);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $1220 + Y($10) = $1230
    //
    m_memory.writeRAM(0x1230, operand);

    clock();
    verifyRead(0x1000, 0xBF);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // No page crossing:
    // operand is read immediately in C4.
    //
    clock();
    verifyRead(0x1230, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x10));

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0x44));

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

    //
    // LAX abs,Y without page crossing = 4 cycles.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedLAX::testAbsoluteYPageCross()
{
    setupCpu();

    const quint8 operand = 0xA5;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x20);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xBF);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    //
    // $12F0 + $20 = $1310.
    // Wrong-page address = $1210.
    //
    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, operand);

    clock();
    verifyRead(0x1000, 0xBF);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // C4: Wrong-page dummy read.
    //
    clock();
    verifyRead(0x1210, 0x5A);

    //
    // C5: Actual operand.
    //
    clock();
    verifyRead(0x1310, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x20));

    QCOMPARE(
        m_cpu.stackPointer(),
        quint8(0x44));

    QCOMPARE(
        m_cpu.status(),
        finalStatus);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1003));

    //
    // Page crossing adds exactly one cycle.
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedLAX::testIndirectX()
{
    setupCpu();

    const quint8 operand = 0xA5;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xA3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20,X)
    // $20 + X($10) -> pointer $30/$31 -> $1234
    //
    m_memory.writeRAM(0x0020, 0x5A);
    m_memory.writeRAM(0x0030, 0x34);
    m_memory.writeRAM(0x0031, 0x12);
    m_memory.writeRAM(0x1234, operand);

    clock();
    verifyRead(0x1000, 0xA3);

    clock();
    verifyRead(0x1001, 0x20);

    //
    // Indexed-indirect dummy read.
    //
    clock();
    verifyRead(0x0020, 0x5A);

    clock();
    verifyRead(0x0030, 0x34);

    clock();
    verifyRead(0x0031, 0x12);

    clock();
    verifyRead(0x1234, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

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
    // LAX (zp,X) = exactly 6 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testIndirectXZeroPageWrap()
{
    setupCpu();

    const quint8 operand = 0x42;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x01);
    m_cpu.setYRegister(0x33);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xA3);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $FE + X($01) -> pointer starts at $FF.
    // Pointer high-byte read must wrap from $FF to $00.
    //
    m_memory.writeRAM(0x00FE, 0x5A);
    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x0000, 0x12);
    m_memory.writeRAM(0x1234, operand);

    clock();
    verifyRead(0x1000, 0xA3);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x5A);

    clock();
    verifyRead(0x00FF, 0x34);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1234, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testIndirectY()
{
    setupCpu();

    const quint8 operand = 0x42;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x10);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xB3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20),Y
    // pointer = $1220
    // $1220 + Y($10) = $1230
    //
    m_memory.writeRAM(0x0020, 0x20);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1230, operand);

    clock();
    verifyRead(0x1000, 0xB3);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x20);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // No page crossing:
    // operand read directly in C5.
    //
    clock();
    verifyRead(0x1230, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x10));

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
    // LAX (zp),Y without crossing = 5 cycles.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testIndirectYPageCross()
{
    setupCpu();

    const quint8 operand = 0xA5;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    const quint8 finalStatus =
        expectedStatus(
            initialStatus,
            expected);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x20);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xB3);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // ($20),Y
    // pointer = $12F0
    // $12F0 + $20 = $1310
    // wrong-page address = $1210
    //
    m_memory.writeRAM(0x0020, 0xF0);
    m_memory.writeRAM(0x0021, 0x12);
    m_memory.writeRAM(0x1210, 0x5A);
    m_memory.writeRAM(0x1310, operand);

    clock();
    verifyRead(0x1000, 0xB3);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xF0);

    clock();
    verifyRead(0x0021, 0x12);

    //
    // C5: Wrong-page dummy read.
    //
    clock();
    verifyRead(0x1210, 0x5A);

    //
    // C6: Actual operand.
    //
    clock();
    verifyRead(0x1310, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    QCOMPARE(
        m_cpu.yRegister(),
        quint8(0x20));

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
    // Page crossing adds exactly one cycle.
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testIndirectYZeroPageWrap()
{
    setupCpu();

    const quint8 operand = 0x42;
    const quint8 initialStatus = 0x21;

    const LAXResult expected =
        referenceLAX(operand);

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x10);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xB3);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // Pointer high byte wraps from $FF to $00.
    // Base address = $1220.
    // Effective address = $1230.
    //
    m_memory.writeRAM(0x00FF, 0x20);
    m_memory.writeRAM(0x0000, 0x12);
    m_memory.writeRAM(0x1230, operand);

    clock();
    verifyRead(0x1000, 0xB3);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x20);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1230, operand);

    QCOMPARE(
        m_cpu.accumulator(),
        expected.accumulator);

    QCOMPARE(
        m_cpu.xRegister(),
        expected.x);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedLAX::testExhaustive()
{
    quint32 testCount = 0;

    for (quint16 operand = 0;
         operand <= 0xFF;
         ++operand)
    {
        setupCpu();

        const quint8 value =
            static_cast<quint8>(
                operand);

        //
        // Deliberately set all flags so that the test
        // verifies that only Z and N are modified.
        //
        const quint8 initialStatus = 0xFF;

        const LAXResult expected =
            referenceLAX(value);

        const quint8 finalStatus =
            expectedStatus(
                initialStatus,
                expected);

        m_cpu.setAccumulator(0x55);
        m_cpu.setXRegister(0xAA);
        m_cpu.setYRegister(0x33);
        m_cpu.setStackPointer(0x44);
        m_cpu.setStatus(initialStatus);
        m_cpu.setProgramCounter(0x1000);

        m_memory.writeRAM(
            0x1000,
            0xA7);

        m_memory.writeRAM(
            0x1001,
            0x80);

        m_memory.writeRAM(
            0x0080,
            value);

        //
        // LAX zp = exactly 3 cycles.
        //
        clock();
        clock();
        clock();

        if (m_cpu.accumulator()
                != expected.accumulator
            || m_cpu.xRegister()
                   != expected.x
            || m_cpu.status()
                   != finalStatus
            || m_cpu.yRegister()
                   != 0x33
            || m_cpu.stackPointer()
                   != 0x44)
        {
            QFAIL(
                qPrintable(
                    QStringLiteral(
                        "LAX mismatch: "
                        "operand=$%1 "
                        "| actual A=$%2 X=$%3 P=$%4 "
                        "| expected A=$%5 X=$%6 P=$%7")
                        .arg(
                            value,
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
                            expected.x,
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

    QCOMPARE(
        testCount,
        quint32(256));
}
