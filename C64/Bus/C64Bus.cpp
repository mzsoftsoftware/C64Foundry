#include "C64Bus.h"

#include "C64/Memory/C64Memory.h"


C64Bus::C64Bus()
{
}
C64Bus::~C64Bus()
{
}

void C64Bus::setMemory(C64Memory* ptrMemory)
{
    m_ptrMemory = ptrMemory;
}

void C64Bus::clock()
{
    m_lastAccessType = AccessType::None;
    m_lastAccessAddress = 0x0000;
    m_lastAccessValue = 0x00;
    m_accessCount = 0;
}

quint8 C64Bus::read(const quint16 address)
{
    const quint8 value = m_ptrMemory->readRAM(address);
    m_lastAccessType = AccessType::Read;
    m_lastAccessAddress = address;
    m_lastAccessValue = value;
    ++m_accessCount;
    return value;
}

void C64Bus::write(const quint16 address,
                   const quint8 value)
{
    m_lastAccessType = AccessType::Write;
    m_lastAccessAddress = address;
    m_lastAccessValue = value;
    ++m_accessCount;
    m_ptrMemory->writeRAM(address, value);
}
