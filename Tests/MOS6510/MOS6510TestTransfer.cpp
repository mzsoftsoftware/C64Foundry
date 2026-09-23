#include "MOS6510TestTransfer.h"

#include <QTest>

MOS6510TestTransfer::MOS6510TestTransfer()
{
}
MOS6510TestTransfer::~MOS6510TestTransfer()
{
}

void MOS6510TestTransfer::initializeRegisters()
{
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0x7D);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestTransfer::testRegisterTransfer_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("sourceValue");
    QTest::addColumn<quint8>("expectedValue");

    QTest::newRow("TAX zero") << quint8(0xAA) << quint8(0x00) << quint8(0x00);
    QTest::newRow("TAX positive") << quint8(0xAA) << quint8(0x42) << quint8(0x42);
    QTest::newRow("TAX negative") << quint8(0xAA) << quint8(0x80) << quint8(0x80);

    QTest::newRow("TAY zero") << quint8(0xA8) << quint8(0x00) << quint8(0x00);
    QTest::newRow("TAY positive") << quint8(0xA8) << quint8(0x42) << quint8(0x42);
    QTest::newRow("TAY negative") << quint8(0xA8) << quint8(0x80) << quint8(0x80);

    QTest::newRow("TXA zero") << quint8(0x8A) << quint8(0x00) << quint8(0x00);
    QTest::newRow("TXA positive") << quint8(0x8A) << quint8(0x42) << quint8(0x42);
    QTest::newRow("TXA negative") << quint8(0x8A) << quint8(0x80) << quint8(0x80);

    QTest::newRow("TYA zero") << quint8(0x98) << quint8(0x00) << quint8(0x00);
    QTest::newRow("TYA positive") << quint8(0x98) << quint8(0x42) << quint8(0x42);
    QTest::newRow("TYA negative") << quint8(0x98) << quint8(0x80) << quint8(0x80);

    QTest::newRow("TSX zero") << quint8(0xBA) << quint8(0x00) << quint8(0x00);
    QTest::newRow("TSX positive") << quint8(0xBA) << quint8(0x42) << quint8(0x42);
    QTest::newRow("TSX negative") << quint8(0xBA) << quint8(0x80) << quint8(0x80);
}
void MOS6510TestTransfer::testRegisterTransfer()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, sourceValue);
    QFETCH(quint8, expectedValue);

    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xEA);
    switch (opcode)
    {
    case 0xAA: // TAX
    case 0xA8: // TAY
        m_cpu.setAccumulator(sourceValue);
        break;
    case 0x8A: // TXA
        m_cpu.setXRegister(sourceValue);
        break;
    case 0x98: // TYA
        m_cpu.setYRegister(sourceValue);
        break;
    case 0xBA: // TSX
        m_cpu.setStackPointer(sourceValue);
        break;
    }

    quint8 expectedAccumulator = 0x11;
    quint8 expectedXRegister = 0x22;
    quint8 expectedYRegister = 0x33;
    quint8 expectedStackPointer = 0x44;
    switch (opcode)
    {
    case 0xAA: // TAX
        expectedAccumulator = sourceValue;
        expectedXRegister = sourceValue;
        break;
    case 0xA8: // TAY
        expectedAccumulator = sourceValue;
        expectedYRegister = sourceValue;
        break;
    case 0x8A: // TXA
        expectedAccumulator = sourceValue;
        expectedXRegister = sourceValue;
        break;
    case 0x98: // TYA
        expectedAccumulator = sourceValue;
        expectedYRegister = sourceValue;
        break;
    case 0xBA: // TSX
        expectedXRegister = sourceValue;
        expectedStackPointer = sourceValue;
        break;
    }
    quint8 expectedStatus = 0x7D;
    if (expectedValue == 0x00)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);
    if (expectedValue & 0x80)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: transfer
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), expectedXRegister);
    QCOMPARE(m_cpu.yRegister(), expectedYRegister);
    QCOMPARE(m_cpu.stackPointer(), expectedStackPointer);
    QCOMPARE(m_cpu.status(), expectedStatus);
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 3: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestTransfer::testTransferXToStackPointer_data()
{
    QTest::addColumn<quint8>("sourceValue");

    QTest::newRow("zero")     << quint8(0x00);
    QTest::newRow("positive") << quint8(0x42);
    QTest::newRow("negative") << quint8(0x80);
}
void MOS6510TestTransfer::testTransferXToStackPointer()
{
    QFETCH(quint8, sourceValue);

    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0x44);
    m_cpu.setXRegister(sourceValue);
    m_memory.writeRAM(0x1000, 0x9A); // TXS
    m_memory.writeRAM(0x1001, 0xEA); // NOP

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: X -> SP
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), sourceValue);
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), sourceValue);
    QCOMPARE(m_cpu.status(), quint8(0x7D));
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 3: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
