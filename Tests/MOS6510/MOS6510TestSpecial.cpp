#include "MOS6510TestSpecial.h"

#include <QTest>

MOS6510TestSpecial::MOS6510TestSpecial()
{
}
MOS6510TestSpecial::~MOS6510TestSpecial()
{
}

void MOS6510TestSpecial::testNOP()
{
    setupCpu();
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStatus(0x7D);
    m_memory.writeRAM(0x1000, 0xEA);
    m_memory.writeRAM(0x1001, 0xEA);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: NOP execution
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 3: Next Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.status(), quint8(0x7D));
}
