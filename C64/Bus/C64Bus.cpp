#include "C64Bus.h"

#include "C64/Memory/C64Memory.h"
#include "C64/VIC-II/VIC-II.h"


C64Bus::C64Bus()
{
    setCpuPortLines(m_cpuPortLines);
}
C64Bus::~C64Bus()
{
}

C64Bus::MemorySource C64Bus::memorySource(const quint16 address) const
{
    return m_ptrMemoryMap[address >> 12];
}

void C64Bus::setCpuPortLines(const quint8 lines)
{
    m_cpuPortLines = lines & 0x07;
    m_ptrMemoryMap = s_memoryMaps[m_cpuPortLines];
}

void C64Bus::setMemory(C64Memory* ptrMemory)
{
    m_ptrMemory = ptrMemory;
}
void C64Bus::setVICII(VICII* ptrVicII)
{
    m_ptrVICII = ptrVicII;
}

void C64Bus::clock()
{
    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::None;
    m_lastAccessSource = AccessSource::None;
    m_lastAccessAddress = 0x0000;
    m_lastAccessValue = 0x00;
    m_accessCount = 0;
}

quint8 C64Bus::read(const quint16 address)
{
    //
    // AEC low disconnects the CPU from the system bus.
    // No CPU read reaches memory or I/O.
    //
    if (!m_aec)
    {
        m_cpuDrivesDataBus = false;
        return m_dataBusValue;
    }

    quint8 value = 0x00;

    const MemorySource source = m_ptrMemoryMap[address >> 12];

    //
    // Fast path for RAM.
    //
    if (source == MemorySource::RAM)
    {
        value = m_ptrMemory->readRAM(address);
    }
    else
    {
        switch (source)
        {
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
            value = 0xFF;

            switch (address & 0x0C00)
            {
            case 0x0000:
                //
                // VIC-II: $D000-$D3FF
                //
                value = m_ptrVICII->readRegister(address & 0x003F);
                break;

            case 0x0400:
                //
                // SID: $D400-$D7FF
                //
                break;

            case 0x0800:
                //
                // Color RAM: $D800-$DBFF
                //
                value = m_ptrMemory->readColorRAM(address & 0x03FF);
                break;

            case 0x0C00:
                switch (address & 0x0300)
                {
                case 0x0000:
                    //
                    // CIA 1: $DC00-$DCFF
                    //
                    break;

                case 0x0100:
                    //
                    // CIA 2: $DD00-$DDFF
                    //
                    break;

                case 0x0200:
                    //
                    // IO1 / Expansion: $DE00-$DEFF
                    //
                    break;

                case 0x0300:
                    //
                    // IO2 / Expansion: $DF00-$DFFF
                    //
                    break;
                }
                break;
            }
            break;

        case MemorySource::RAM:
            //
            // RAM is handled by the fast path above.
            //
            break;
        }
    }

    m_dataBusValue = value;
    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::Read;
    m_lastAccessSource = AccessSource::CPU;
    m_lastAccessAddress = address;
    m_lastAccessValue = value;
    ++m_accessCount;

    return value;
}

void C64Bus::write(const quint16 address, const quint8 value)
{
    m_lastAccessType = AccessType::Write;
    m_lastAccessSource = AccessSource::CPU;
    m_lastAccessAddress = address;
    m_lastAccessValue = value;
    ++m_accessCount;

    m_dataBusValue = value;
    m_cpuDrivesDataBus = m_aec;

    //
    // AEC low disconnects the CPU from the system bus.
    // The CPU write must not reach memory or I/O.
    //
    if (!m_aec)
        return;

    const MemorySource source = m_ptrMemoryMap[address >> 12];

    //
    // Fast path for RAM and RAM below ROM.
    //
    if (source != MemorySource::IO)
    {
        m_ptrMemory->writeRAM(address, value);
        return;
    }

    //
    // I/O is visible at $D000-$DFFF.
    //
    switch (address & 0x0C00)
    {
    case 0x0000:
        //
        // VIC-II: $D000-$D3FF
        //
        m_ptrVICII->writeRegister(address & 0x003F, value);
        break;

    case 0x0400:
        //
        // SID: $D400-$D7FF
        //
        break;

    case 0x0800:
        //
        // Color RAM: $D800-$DBFF
        //
        m_ptrMemory->writeColorRAM(address & 0x03FF, value);
        break;

    case 0x0C00:
        switch (address & 0x0300)
        {
        case 0x0000:
            //
            // CIA 1: $DC00-$DCFF
            //
            break;

        case 0x0100:
            //
            // CIA 2: $DD00-$DDFF
            //
            break;

        case 0x0200:
            //
            // IO1 / Expansion: $DE00-$DEFF
            //
            break;

        case 0x0300:
            //
            // IO2 / Expansion: $DF00-$DFFF
            //
            break;
        }
        break;
    }
}

void C64Bus::readCycle(const quint16 address)
{
    //
    // AEC low disconnects the CPU from the system bus.
    // No CPU read reaches RAM.
    //
    if (!m_aec)
    {
        m_cpuDrivesDataBus = false;
        return;
    }

    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::Read;
    m_lastAccessSource = AccessSource::CPU;
    m_lastAccessAddress = address;
    ++m_accessCount;

    m_dataBusValue = m_ptrMemory->readRAM(address);
    m_lastAccessValue = m_dataBusValue;
}
void C64Bus::writeCycle(const quint16 address)
{
    //
    // AEC low disconnects the CPU from the system bus.
    // No CPU write reaches RAM.
    //
    if (!m_aec)
    {
        m_cpuDrivesDataBus = false;
        return;
    }

    m_cpuDrivesDataBus = false;

    m_lastAccessType = AccessType::Write;
    m_lastAccessSource = AccessSource::CPU;
    m_lastAccessAddress = address;
    ++m_accessCount;

    m_ptrMemory->writeRAM(address, m_dataBusValue);
}

quint8 C64Bus::readVIC(const quint16 address)
{
    const quint16 vicAddress = address & 0x3FFF;
    quint8 value;

    //
    // In VIC-II bank 0, $1000-$1FFF is mapped to the
    // Character ROM instead of the underlying RAM.
    //
    if ((vicAddress & 0x3000) == 0x1000)
        value = m_ptrMemory->readCharacterROM(vicAddress & 0x0FFF);
    else
        value = m_ptrMemory->readRAM(vicAddress);

    //
    // Record the VIC-II bus access.
    //
    m_lastAccessType = AccessType::Read;
    m_lastAccessSource = AccessSource::VICII;
    m_lastAccessAddress = vicAddress;
    m_lastAccessValue = value;
    ++m_accessCount;

    return value;
}