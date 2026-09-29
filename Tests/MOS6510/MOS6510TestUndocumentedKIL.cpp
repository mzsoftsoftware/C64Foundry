#include "MOS6510TestUndocumentedKIL.h"

#include <QTest>


MOS6510TestUndocumentedKIL::MOS6510TestUndocumentedKIL()
{
}

MOS6510TestUndocumentedKIL::~MOS6510TestUndocumentedKIL()
{
}


void MOS6510TestUndocumentedKIL::testKIL_data()
{
    QTest::addColumn<quint8>("opcode");

    //
    // All twelve NMOS 6502/6510 KIL/JAM opcodes.
    //
    QTest::newRow("02") << quint8(0x02);
    QTest::newRow("12") << quint8(0x12);
    QTest::newRow("22") << quint8(0x22);
    QTest::newRow("32") << quint8(0x32);
    QTest::newRow("42") << quint8(0x42);
    QTest::newRow("52") << quint8(0x52);
    QTest::newRow("62") << quint8(0x62);
    QTest::newRow("72") << quint8(0x72);
    QTest::newRow("92") << quint8(0x92);
    QTest::newRow("B2") << quint8(0xB2);
    QTest::newRow("D2") << quint8(0xD2);
    QTest::newRow("F2") << quint8(0xF2);
}


void MOS6510TestUndocumentedKIL::testKIL()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);

    m_memory.writeRAM(0x1000, opcode);

    //
    // Put a valid instruction behind KIL.
    //
    // It must never be fetched.
    //
    m_memory.writeRAM(0x1001, 0xEA);

    QVERIFY(!m_cpu.stopped());

    //
    // C1:
    //
    // Fetch KIL.
    //
    clock();

    verifyRead(
        0x1000,
        opcode);

    //
    // KIL stops the CPU immediately after its
    // opcode fetch.
    //
    QVERIFY(m_cpu.stopped());

    //
    // The opcode fetch incremented PC once.
    //
    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    //
    // KIL itself does not modify the registers
    // or processor status.
    //
    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x11));

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
        quint8(0xA5));

    //
    // C2:
    //
    // Once stopped, the CPU must perform no
    // further bus access.
    //
    clock();

    verifyNoAccess();

    QVERIFY(m_cpu.stopped());

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x1001));

    QCOMPARE(
        m_cpu.accumulator(),
        quint8(0x11));

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
        quint8(0xA5));
}


void MOS6510TestUndocumentedKIL::testKILRemainsStopped()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setAccumulator(0x12);
    m_cpu.setXRegister(0x34);
    m_cpu.setYRegister(0x56);
    m_cpu.setStackPointer(0x78);
    m_cpu.setStatus(0xA9);

    //
    // Use one representative KIL encoding.
    //
    m_memory.writeRAM(0x2000, 0x02);

    //
    // Fill the following bytes with valid instructions.
    // None of them may ever be fetched.
    //
    m_memory.writeRAM(0x2001, 0xEA);
    m_memory.writeRAM(0x2002, 0xEA);
    m_memory.writeRAM(0x2003, 0xEA);

    //
    // Fetch KIL.
    //
    clock();

    verifyRead(
        0x2000,
        0x02);

    QVERIFY(m_cpu.stopped());

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x2001));

    //
    // The CPU must remain permanently stopped.
    //
    // Run several additional clocks and verify the
    // complete externally observable CPU state after
    // every single one.
    //
    for (quint8 cycle = 0;
         cycle < 16;
         ++cycle)
    {
        clock();

        verifyNoAccess();

        QVERIFY(m_cpu.stopped());

        QCOMPARE(
            m_cpu.programCounter(),
            quint16(0x2001));

        QCOMPARE(
            m_cpu.accumulator(),
            quint8(0x12));

        QCOMPARE(
            m_cpu.xRegister(),
            quint8(0x34));

        QCOMPARE(
            m_cpu.yRegister(),
            quint8(0x56));

        QCOMPARE(
            m_cpu.stackPointer(),
            quint8(0x78));

        QCOMPARE(
            m_cpu.status(),
            quint8(0xA9));
    }
}
