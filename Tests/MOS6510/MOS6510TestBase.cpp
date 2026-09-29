#include "MOS6510TestBase.h"

#include <QTest>


MOS6510TestBase::MOS6510TestBase()
{
}
MOS6510TestBase::~MOS6510TestBase()
{
}

void MOS6510TestBase::setupCpu()
{
    m_bus.setMemory(&m_memory);
    m_cpu.setBus(&m_bus);
    m_cpu.initialize();
}

void MOS6510TestBase::clock()
{
    m_bus.clock();
    m_cpu.clock();
}
void MOS6510TestBase::verifyRead(const quint16 address, const quint8 value)
{
    QCOMPARE(m_bus.accessCount(), quint8(1));
    QCOMPARE(m_bus.lastAccessType(), C64Bus::AccessType::Read);
    QCOMPARE(m_bus.lastAccessAddress(), address);
    QCOMPARE(m_bus.lastAccessValue(), value);
}
void MOS6510TestBase::verifyWrite(const quint16 address, const quint8 value)
{
    QCOMPARE(m_bus.accessCount(), quint8(1));
    QCOMPARE(m_bus.lastAccessType(), C64Bus::AccessType::Write);
    QCOMPARE(m_bus.lastAccessAddress(), address);
    QCOMPARE(m_bus.lastAccessValue(), value);
}
void MOS6510TestBase::verifyNoAccess()
{
    QCOMPARE(m_bus.accessCount(), quint8(0));
    QCOMPARE(m_bus.lastAccessType(), C64Bus::AccessType::None);
}

void MOS6510TestBase::setDataDirectionRegister(const quint8 value)
{
    m_cpu.setProgramCounter(0x2000);
    m_cpu.setAccumulator(value);

    m_memory.writeRAM(0x2000, 0x85);    // STA $00
    m_memory.writeRAM(0x2001, 0x00);

    clock();
    clock();
    clock();
}