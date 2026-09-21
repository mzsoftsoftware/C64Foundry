#include "C64Machine.h"

#include "Bus/C64Bus.h"
#include "Memory/C64Memory.h"
#include "CPU/MOS6510.h"


C64Machine::C64Machine()
{
    m_ptrBus = new C64Bus();
    m_ptrMemory = new C64Memory();

    m_ptrBus->setMemory(m_ptrMemory);

    m_ptrCpu = new MOS6510();
    m_ptrCpu->setBus(m_ptrBus);
}
C64Machine::~C64Machine()
{
    delete m_ptrCpu;
    delete m_ptrMemory;
    delete m_ptrBus;
}

