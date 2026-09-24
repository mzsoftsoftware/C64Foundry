#include "MOS6510TestIncrementDecrement.h"

#include <QTest>


MOS6510TestIncrementDecrement::MOS6510TestIncrementDecrement()
{
}
MOS6510TestIncrementDecrement::~MOS6510TestIncrementDecrement()
{
}

void MOS6510TestIncrementDecrement::initializeRegisters()
{
    m_cpu.setAccumulator(0x55);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0xFF);

    // N V - B D I Z C
    // 0 1 1 1 1 1 0 1
    //
    // INX, INY, DEX and DEY may only modify N and Z.
    // V, B, D, I and C are deliberately set so that the tests
    // also verify that these flags remain unchanged.
    m_cpu.setStatus(0x7D);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestIncrementDecrement::testInx_data()
{
    QTest::addColumn<quint8>("initialValue");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("zero to positive") << quint8(0x00) << quint8(0x01) << quint8(0x7D);
    QTest::newRow("positive to negative") << quint8(0x7F) << quint8(0x80) << quint8(0xFD);
    QTest::newRow("negative") << quint8(0x80) << quint8(0x81) << quint8(0xFD);
    QTest::newRow("wrap to zero") << quint8(0xFF) << quint8(0x00) << quint8(0x7F);
}

void MOS6510TestIncrementDecrement::testInx()
{
    QFETCH(quint8, initialValue);
    QFETCH(quint8, expectedValue);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setXRegister(initialValue);
    m_memory.writeRAM(0x1000, 0xE8); // INX
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), initialValue);
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: INX
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.xRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestIncrementDecrement::testIny_data()
{
    QTest::addColumn<quint8>("initialValue");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("zero to positive") << quint8(0x00) << quint8(0x01) << quint8(0x7D);
    QTest::newRow("positive to negative") << quint8(0x7F) << quint8(0x80) << quint8(0xFD);
    QTest::newRow("negative") << quint8(0x80) << quint8(0x81) << quint8(0xFD);
    QTest::newRow("wrap to zero") << quint8(0xFF) << quint8(0x00) << quint8(0x7F);
}

void MOS6510TestIncrementDecrement::testIny()
{
    QFETCH(quint8, initialValue);
    QFETCH(quint8, expectedValue);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(initialValue);
    m_memory.writeRAM(0x1000, 0xC8); // INY
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), initialValue);
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: INY
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.yRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestIncrementDecrement::testDex_data()
{
    QTest::addColumn<quint8>("initialValue");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("wrap to negative") << quint8(0x00) << quint8(0xFF) << quint8(0xFD);
    QTest::newRow("positive to zero") << quint8(0x01) << quint8(0x00) << quint8(0x7F);
    QTest::newRow("positive") << quint8(0x7F) << quint8(0x7E) << quint8(0x7D);
    QTest::newRow("negative to positive") << quint8(0x80) << quint8(0x7F) << quint8(0x7D);
}

void MOS6510TestIncrementDecrement::testDex()
{
    QFETCH(quint8, initialValue);
    QFETCH(quint8, expectedValue);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setXRegister(initialValue);
    m_memory.writeRAM(0x1000, 0xCA); // DEX
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), initialValue);
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: DEX
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.xRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestIncrementDecrement::testDey_data()
{
    QTest::addColumn<quint8>("initialValue");
    QTest::addColumn<quint8>("expectedValue");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("wrap to negative") << quint8(0x00) << quint8(0xFF) << quint8(0xFD);
    QTest::newRow("positive to zero") << quint8(0x01) << quint8(0x00) << quint8(0x7F);
    QTest::newRow("positive") << quint8(0x7F) << quint8(0x7E) << quint8(0x7D);
    QTest::newRow("negative to positive") << quint8(0x80) << quint8(0x7F) << quint8(0x7D);
}

void MOS6510TestIncrementDecrement::testDey()
{
    QFETCH(quint8, initialValue);
    QFETCH(quint8, expectedValue);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(initialValue);
    m_memory.writeRAM(0x1000, 0x88); // DEY
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), initialValue);
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: DEY
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.yRegister(), expectedValue);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
