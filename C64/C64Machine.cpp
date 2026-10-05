#include "C64Machine.h"

#include <QDebug>

#include "C64ROMSet.h"
#include "Bus/C64Bus.h"
#include "Memory/C64Memory.h"
#include "CPU/MOS6510.h"
#include "VIC-II/VIC-II.h"
#include "CIA/MOS6526.h"


C64Machine::C64Machine()
{
    m_ptrBus = new C64Bus();
    m_ptrMemory = new C64Memory();
    m_ptrBus->setMemory(m_ptrMemory);

    m_ptrVICII = new VICII();
    m_ptrVICII->setBus(m_ptrBus);
    m_ptrVICII->setTiming(m_timing);
    m_ptrBus->setVICII(m_ptrVICII);

    m_ptrCIA1 = new MOS6526();
    m_ptrBus->setCIA1(m_ptrCIA1);

    m_ptrCIA2 = new MOS6526();
    m_ptrBus->setCIA2(m_ptrCIA2);

    m_ptrCpu = new MOS6510();
    m_ptrCpu->setBus(m_ptrBus);
}
C64Machine::~C64Machine()
{
    delete m_ptrCpu;
    delete m_ptrCIA2;
    delete m_ptrCIA1;
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
bool C64Machine::loadProgram(const quint16 address, const QByteArray& data)
{
    if (data.size() > (0x10000 - address))
        return false;
    for (qsizetype index = 0; index < data.size(); ++index)
    {
        m_ptrMemory->writeRAM(static_cast<quint16>(address + index), static_cast<quint8>(data.at(index)));
    }
    return true;
}

void C64Machine::powerOn()
{
    m_cycles = 0;

    m_ptrCpu->initialize();
    m_ptrCpu->reset();
}
void C64Machine::reset()
{
    m_cycles = 0;

    m_ptrCpu->reset();
}

void C64Machine::clock()
{
    ++m_cycles;

    m_ptrBus->clock();

    m_ptrVICII->clock();

    //
    // The VIC-II controls which chip owns the system bus.
    //
    m_ptrBus->setAEC(m_ptrVICII->aec());

    //
    // The VIC-II drives the CPU IRQ line.
    //
    m_ptrCpu->setIrqLine(m_ptrVICII->irq());

    //
    // BA low warns the CPU that the VIC-II needs the bus.
    // CPU read cycles are stalled while write cycles may
    // still complete.
    //
    if (m_ptrVICII->ba())
        m_ptrCpu->clock();
    else
        m_ptrCpu->clockReadyLow();
}

void C64Machine::runCycles(const quint64 cycles)
{
    for (quint64 cycle = 0; cycle < cycles; ++cycle)
        clock();
}

void C64Machine::setTiming(const C64::Timing& timing)
{
    m_timing = timing;
    m_ptrVICII->setTiming(timing);
}
void C64Machine::writeVICIIRegister(const quint8 address, const quint8 value)
{
    m_ptrVICII->writeRegister(address, value);
}
void C64Machine::writeCIA1Register(const quint8 address, const quint8 value)
{
    m_ptrCIA1->writeRegister(address, value);
}

void C64Machine::writeCIA2Register(const quint8 address, const quint8 value)
{
    m_ptrCIA2->writeRegister(address, value);
}

quint8 C64Machine::readRAM(const quint16 address) const
{
    return m_ptrMemory->readRAM(address);
}
bool C64Machine::viciiBA() const
{
    return m_ptrVICII->ba();
}
bool C64Machine::viciiAEC() const
{
    return m_ptrVICII->aec();
}
quint8 C64Machine::readCIA1Register(const quint8 address) const
{
    return m_ptrCIA1->readRegister(address);
}
quint8 C64Machine::readCIA2Register(const quint8 address) const
{
    return m_ptrCIA2->readRegister(address);
}
bool C64Machine::busAEC() const
{
    return m_ptrBus->aec();
}
quint8 C64Machine::busAccessCount() const
{
    return m_ptrBus->accessCount();
}
bool C64Machine::busLastAccessWasRead() const
{
    return m_ptrBus->lastAccessType() == C64Bus::AccessType::Read;
}
bool C64Machine::busLastAccessWasWrite() const
{
    return m_ptrBus->lastAccessType() == C64Bus::AccessType::Write;
}
quint16 C64Machine::busLastAccessAddress() const
{
    return m_ptrBus->lastAccessAddress();
}
quint8 C64Machine::busLastAccessValue() const
{
    return m_ptrBus->lastAccessValue();
}

const quint8* C64Machine::acquireVideoFrame()
{
    return m_ptrVICII->acquireReadyFrame();
}
const quint8* C64Machine::peekVideoFrame() const
{
    return m_ptrVICII->peekReadyFrame();
}
quint16 C64Machine::videoFrameWidth() const
{
    return m_ptrVICII->frameWidth();
}
quint16 C64Machine::videoFrameHeight() const
{
    return m_ptrVICII->frameHeight();
}