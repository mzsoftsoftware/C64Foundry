#include "C64Bus.h"

#include "C64/Memory/C64Memory.h"


C64Bus::C64Bus()
{
}
C64Bus::~C64Bus()
{
}

C64Bus::MemorySource C64Bus::memorySource(const quint16 address) const
{
    if (address >= 0xA000 && address <= 0xBFFF)
        return m_basicSource;

    if (address >= 0xD000 && address <= 0xDFFF)
        return m_ioSource;

    if (address >= 0xE000)
        return m_kernalSource;

    return MemorySource::RAM;
}

void C64Bus::setCpuPortLines(const quint8 lines)
{
    m_cpuPortLines = lines & 0x07;

    m_basicSource = s_basicMapping[m_cpuPortLines];
    m_ioSource = s_ioMapping[m_cpuPortLines];
    m_kernalSource = s_kernalMapping[m_cpuPortLines];
}
void C64Bus::setMemory(C64Memory* ptrMemory)
{
    m_ptrMemory = ptrMemory;
}

void C64Bus::clock()
{
    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::None;
    m_lastAccessAddress = 0x0000;
    m_lastAccessValue = 0x00;
    m_accessCount = 0;
}

quint8 C64Bus::read(const quint16 address)
{
    quint8 value = 0xFF;
    switch (memorySource(address))
    {
    case MemorySource::RAM:
        value = m_ptrMemory->readRAM(address);
        break;
    case MemorySource::BasicROM:
        value = m_ptrMemory->readBasicROM(address - 0xA000);
        break;
    case MemorySource::KernalROM:
        value = m_ptrMemory->readKernalROM(address - 0xE000);
        break;
    case MemorySource::CharacterROM:
        value = m_ptrMemory->readCharacterROM(address - 0xD000);
        break;
    case MemorySource::IO:
        //
        // I/O mapping will be implemented next.
        //
        break;
    }

    m_dataBusValue = value;
    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::Read;
    m_lastAccessAddress = address;
    m_lastAccessValue = value;
    ++m_accessCount;

    return value;
}

void C64Bus::write(const quint16 address, const quint8 value)
{
    m_lastAccessType = AccessType::Write;
    m_lastAccessAddress = address;
    m_lastAccessValue = value;
    ++m_accessCount;

    m_dataBusValue = value;
    m_cpuDrivesDataBus = true;

    m_ptrMemory->writeRAM(address, value);
}

void C64Bus::readCycle(const quint16 address)
{
    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::Read;
    m_lastAccessAddress = address;
    ++m_accessCount;

    m_dataBusValue = m_ptrMemory->readRAM(address);
    m_lastAccessValue = m_dataBusValue;
}
void C64Bus::writeCycle(const quint16 address)
{
    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::Write;
    m_lastAccessAddress = address;
    ++m_accessCount;

    m_ptrMemory->writeRAM(address, m_dataBusValue);
}
