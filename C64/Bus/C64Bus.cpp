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

quint8 C64Bus::read(const quint16 address) const
{
    return m_ptrMemory->readRAM(address);
}

void C64Bus::write(const quint16 address,
                   const quint8 value)
{
    m_ptrMemory->writeRAM(address, value);
}