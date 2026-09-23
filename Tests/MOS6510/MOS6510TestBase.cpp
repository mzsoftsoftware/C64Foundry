#include "MOS6510TestBase.h"


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
    m_cpu.reset();
}
