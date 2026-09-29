#include "MOS6510TestUndocumentedSAX.h"

#include <QTest>


MOS6510TestUndocumentedSAX::MOS6510TestUndocumentedSAX()
{
}
MOS6510TestUndocumentedSAX::~MOS6510TestUndocumentedSAX()
{
}


// -----------------------------------------------------------------------------
// $87 SAX Zero Page
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSAX::testZeroPage_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint8>("expected");

    QTest::newRow("both zero")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x00);

    QTest::newRow("both ff")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0xFF);

    QTest::newRow("alternating")
        << quint8(0xAA)
        << quint8(0x55)
        << quint8(0x00);

    QTest::newRow("partial")
        << quint8(0xF3)
        << quint8(0x5F)
        << quint8(0x53);

    QTest::newRow("high bit")
        << quint8(0xC0)
        << quint8(0x80)
        << quint8(0x80);
}


void MOS6510TestUndocumentedSAX::testZeroPage()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, xRegister);
    QFETCH(quint8, expected);

    setupCpu();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(xRegister);
    m_cpu.setYRegister(0x37);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xFD);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x87);
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0042, 0xCC);

    //
    // Cycle 1: opcode fetch
    //
    clock();
    verifyRead(0x1000, 0x87);

    //
    // Cycle 2: zero-page address
    //
    clock();
    verifyRead(0x1001, 0x42);

    //
    // Cycle 3: store A & X
    //
    clock();
    verifyWrite(0x0042, expected);

    QCOMPARE(m_memory.readRAM(0x0042), expected);

    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), xRegister);
    QCOMPARE(m_cpu.yRegister(), quint8(0x37));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xA5));
    QCOMPARE(m_cpu.status(), quint8(0xFD));

    //
    // Cycle 4: next opcode fetch proves exact 3-cycle timing
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// $97 SAX Zero Page,Y
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSAX::testZeroPageY_data()
{
    QTest::addColumn<quint8>("zeroPage");
    QTest::addColumn<quint8>("yRegister");
    QTest::addColumn<quint16>("expectedAddress");

    QTest::newRow("normal")
        << quint8(0x20)
        << quint8(0x10)
        << quint16(0x0030);

    QTest::newRow("upper zero page")
        << quint8(0x80)
        << quint8(0x40)
        << quint16(0x00C0);
}


void MOS6510TestUndocumentedSAX::testZeroPageY()
{
    QFETCH(quint8, zeroPage);
    QFETCH(quint8, yRegister);
    QFETCH(quint16, expectedAddress);

    setupCpu();

    m_cpu.setAccumulator(0xF3);
    m_cpu.setXRegister(0x5F);
    m_cpu.setYRegister(yRegister);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xFD);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x97);
    m_memory.writeRAM(0x1001, zeroPage);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(expectedAddress, 0xCC);

    //
    // Cycle 1: opcode fetch
    //
    clock();
    verifyRead(0x1000, 0x97);

    //
    // Cycle 2: zero-page base address
    //
    clock();
    verifyRead(0x1001, zeroPage);

    //
    // Cycle 3: indexed zero-page dummy read
    //
    clock();
    verifyRead(zeroPage, m_memory.readRAM(zeroPage));

    //
    // Cycle 4: store A & X
    //
    clock();
    verifyWrite(expectedAddress, 0x53);

    QCOMPARE(m_memory.readRAM(expectedAddress), quint8(0x53));

    QCOMPARE(m_cpu.accumulator(), quint8(0xF3));
    QCOMPARE(m_cpu.xRegister(), quint8(0x5F));
    QCOMPARE(m_cpu.yRegister(), yRegister);
    QCOMPARE(m_cpu.stackPointer(), quint8(0xA5));
    QCOMPARE(m_cpu.status(), quint8(0xFD));

    //
    // Cycle 5: next opcode fetch proves exact 4-cycle timing
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedSAX::testZeroPageYWrap()
{
    setupCpu();

    m_cpu.setAccumulator(0xFC);
    m_cpu.setXRegister(0x3F);
    m_cpu.setYRegister(0x20);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xFD);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x97);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x00F0, 0x66);
    m_memory.writeRAM(0x0010, 0xCC);

    clock();
    verifyRead(0x1000, 0x97);

    clock();
    verifyRead(0x1001, 0xF0);

    //
    // Dummy read uses the unindexed zero-page address.
    //
    clock();
    verifyRead(0x00F0, 0x66);

    //
    // $F0 + $20 wraps to $10.
    //
    clock();
    verifyWrite(0x0010, 0x3C);

    QCOMPARE(m_memory.readRAM(0x0010), quint8(0x3C));

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// $8F SAX Absolute
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSAX::testAbsolute_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint8>("expected");

    QTest::newRow("zero")
        << quint8(0x00)
        << quint8(0xFF)
        << quint8(0x00);

    QTest::newRow("all bits")
        << quint8(0xFF)
        << quint8(0xFF)
        << quint8(0xFF);

    QTest::newRow("mixed")
        << quint8(0xA7)
        << quint8(0x3C)
        << quint8(0x24);
}


void MOS6510TestUndocumentedSAX::testAbsolute()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, xRegister);
    QFETCH(quint8, expected);

    setupCpu();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(xRegister);
    m_cpu.setYRegister(0x37);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xFD);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x8F);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(0x1234, 0xCC);

    //
    // Cycle 1: opcode fetch
    //
    clock();
    verifyRead(0x1000, 0x8F);

    //
    // Cycle 2: absolute address low
    //
    clock();
    verifyRead(0x1001, 0x34);

    //
    // Cycle 3: absolute address high
    //
    clock();
    verifyRead(0x1002, 0x12);

    //
    // Cycle 4: store A & X
    //
    clock();
    verifyWrite(0x1234, expected);

    QCOMPARE(m_memory.readRAM(0x1234), expected);

    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), xRegister);
    QCOMPARE(m_cpu.yRegister(), quint8(0x37));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xA5));
    QCOMPARE(m_cpu.status(), quint8(0xFD));

    //
    // Cycle 5: next opcode fetch proves exact 4-cycle timing
    //
    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// $83 SAX (Indirect,X)
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSAX::testIndexedIndirect_data()
{
    QTest::addColumn<quint8>("zeroPage");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint8>("pointer");
    QTest::addColumn<quint16>("target");

    QTest::newRow("normal")
        << quint8(0x20)
        << quint8(0x10)
        << quint8(0x30)
        << quint16(0x4567);

    QTest::newRow("indexed wrap")
        << quint8(0xF0)
        << quint8(0x20)
        << quint8(0x10)
        << quint16(0x89AB);
}


void MOS6510TestUndocumentedSAX::testIndexedIndirect()
{
    QFETCH(quint8, zeroPage);
    QFETCH(quint8, xRegister);
    QFETCH(quint8, pointer);
    QFETCH(quint16, target);

    setupCpu();

    m_cpu.setAccumulator(0xF3);
    m_cpu.setXRegister(xRegister);
    m_cpu.setYRegister(0x37);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xFD);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x83);
    m_memory.writeRAM(0x1001, zeroPage);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(zeroPage, 0x66);

    m_memory.writeRAM(
        pointer,
        static_cast<quint8>(target & 0x00FF));

    m_memory.writeRAM(
        static_cast<quint8>(pointer + 1),
        static_cast<quint8>(target >> 8));

    m_memory.writeRAM(target, 0xCC);

    const quint8 expected =
        static_cast<quint8>(0xF3 & xRegister);

    //
    // Cycle 1: opcode fetch
    //
    clock();
    verifyRead(0x1000, 0x83);

    //
    // Cycle 2: zero-page operand
    //
    clock();
    verifyRead(0x1001, zeroPage);

    //
    // Cycle 3: indexed-indirect dummy read
    //
    clock();
    verifyRead(zeroPage, 0x66);

    //
    // Cycle 4: pointer low
    //
    clock();
    verifyRead(
        pointer,
        static_cast<quint8>(target & 0x00FF));

    //
    // Cycle 5: pointer high
    //
    clock();
    verifyRead(
        static_cast<quint8>(pointer + 1),
        static_cast<quint8>(target >> 8));

    //
    // Cycle 6: store A & X
    //
    clock();
    verifyWrite(target, expected);

    QCOMPARE(m_memory.readRAM(target), expected);

    QCOMPARE(m_cpu.accumulator(), quint8(0xF3));
    QCOMPARE(m_cpu.xRegister(), xRegister);
    QCOMPARE(m_cpu.yRegister(), quint8(0x37));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xA5));
    QCOMPARE(m_cpu.status(), quint8(0xFD));

    //
    // Cycle 7: next opcode fetch proves exact 6-cycle timing
    //
    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedSAX::testIndexedIndirectPointerWrap()
{
    setupCpu();

    //
    // Pointer high byte after zero-page wrap is read from $0000,
    // which is the MOS6510 data-direction register.
    //
    setDataDirectionRegister(0x12);

    //
    // Restore the complete CPU state required by this test.
    //
    m_cpu.setAccumulator(0xF3);
    m_cpu.setXRegister(0x0F);
    m_cpu.setYRegister(0x37);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0xFD);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x83);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F0 + X($0F) = $FF.
    //
    m_memory.writeRAM(0x00F0, 0x66);

    //
    // Pointer low at $FF, pointer high wraps to $00.
    //
    m_memory.writeRAM(0x00FF, 0x34);

    m_memory.writeRAM(0x1234, 0xCC);

    clock();
    verifyRead(0x1000, 0x83);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x00F0, 0x66);

    clock();
    verifyRead(0x00FF, 0x34);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyWrite(0x1234, 0x03);

    QCOMPARE(m_memory.readRAM(0x1234), quint8(0x03));

    QCOMPARE(m_cpu.accumulator(), quint8(0xF3));
    QCOMPARE(m_cpu.xRegister(), quint8(0x0F));
    QCOMPARE(m_cpu.yRegister(), quint8(0x37));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xA5));
    QCOMPARE(m_cpu.status(), quint8(0xFD));

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// Exhaustive semantics
//
// Test every possible combination of A and X.
//
// SAX:
//     M = A & X
//
// No register or status flag is modified.
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSAX::testExhaustive()
{
    setupCpu();

    m_memory.writeRAM(0x1000, 0x87);
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    for (quint16 accumulator = 0;
         accumulator <= 0xFF;
         ++accumulator)
    {
        for (quint16 xRegister = 0;
             xRegister <= 0xFF;
             ++xRegister)
        {
            const quint8 a =
                static_cast<quint8>(accumulator);

            const quint8 x =
                static_cast<quint8>(xRegister);

            const quint8 expected =
                static_cast<quint8>(a & x);

            //
            // Use a status value with every writable flag set so
            // accidental flag modifications are immediately visible.
            //
            const quint8 initialStatus = 0xFD;

            m_cpu.initialize();

            m_cpu.setAccumulator(a);
            m_cpu.setXRegister(x);
            m_cpu.setYRegister(0x37);
            m_cpu.setStackPointer(0xA5);
            m_cpu.setStatus(initialStatus);
            m_cpu.setProgramCounter(0x1000);

            m_memory.writeRAM(0x0042, 0xCC);

            //
            // Exact SAX zp timing: 3 cycles.
            //
            clock();
            verifyRead(0x1000, 0x87);

            clock();
            verifyRead(0x1001, 0x42);

            clock();
            verifyWrite(0x0042, expected);

            if (m_memory.readRAM(0x0042) != expected
                || m_cpu.accumulator() != a
                || m_cpu.xRegister() != x
                || m_cpu.yRegister() != 0x37
                || m_cpu.stackPointer() != 0xA5
                || m_cpu.status() != initialStatus)
            {
                QFAIL(
                    qPrintable(
                        QStringLiteral(
                            "SAX mismatch: A=$%1 X=$%2 | "
                            "actual M=$%3 A=$%4 X=$%5 P=$%6 | "
                            "expected M=$%7 P=$%8")
                            .arg(a, 2, 16, QLatin1Char('0'))
                            .arg(x, 2, 16, QLatin1Char('0'))
                            .arg(
                                m_memory.readRAM(0x0042),
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
                                expected,
                                2,
                                16,
                                QLatin1Char('0'))
                            .arg(
                                initialStatus,
                                2,
                                16,
                                QLatin1Char('0'))));
            }

            //
            // Fetch of the next opcode proves that SAX ended
            // after exactly three cycles.
            //
            clock();
            verifyRead(0x1002, 0xEA);
        }
    }
}
