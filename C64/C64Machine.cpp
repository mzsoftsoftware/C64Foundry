#include "C64Machine.h"

#include <QDebug>

#include "C64ROMSet.h"
#include "Bus/C64Bus.h"
#include "Memory/C64Memory.h"
#include "CPU/MOS6510.h"
#include "VIC-II/VIC-II.h"


C64Machine::C64Machine()
{
    m_ptrBus = new C64Bus();
    m_ptrMemory = new C64Memory();

    m_ptrBus->setMemory(m_ptrMemory);

    m_ptrVICII = new VICII();
    m_ptrVICII->setTiming(m_timing);
    m_ptrBus->setVICII(m_ptrVICII);

    m_ptrCpu = new MOS6510();
    m_ptrCpu->setBus(m_ptrBus);
}
C64Machine::~C64Machine()
{
    delete m_ptrCpu;
    delete m_ptrVICII;
    delete m_ptrMemory;
    delete m_ptrBus;
}

bool C64Machine::loadROMSet(const C64ROMSet& romSet)
{
    //
    // Validate the complete ROM set before changing the machine state.
    //
    if (!romSet.isValid())
        return false;

    if (!m_ptrMemory->loadBasicROM(romSet.basicROMFileName))
        return false;

    if (!m_ptrMemory->loadKernalROM(romSet.kernalROMFileName))
        return false;

    if (!m_ptrMemory->loadCharacterROM(romSet.characterROMFileName))
        return false;

    return true;
}

void C64Machine::powerOn()
{
}
void C64Machine::reset()
{
    m_cycles = 0;
}

void C64Machine::clock()
{
    ++m_cycles;

    m_ptrBus->clock();
    m_ptrVICII->clock();
    m_ptrCpu->clock();
}

void C64Machine::setTiming(const C64::Timing& timing)
{
    m_timing = timing;
    m_ptrVICII->setTiming(timing);
}
