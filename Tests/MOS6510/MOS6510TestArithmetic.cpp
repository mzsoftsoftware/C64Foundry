#include "MOS6510TestArithmetic.h"

#include <QTest>


MOS6510TestArithmetic::MOS6510TestArithmetic()
{
}
MOS6510TestArithmetic::~MOS6510TestArithmetic()
{
}

void MOS6510TestArithmetic::initializeRegisters()
{
    m_cpu.setAccumulator(0x55);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x7D);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x10) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34);
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34);
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4);
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77);
}

void MOS6510TestArithmetic::testAdcImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(static_cast<quint8>(0x74 | carryIn));
    m_memory.writeRAM(0x1000, 0x69);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: ADC immediate
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

