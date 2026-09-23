#include "C64Memory.h"

#include "RAM.h"
#include "BasicROM.h"
#include "KernalROM.h"
#include "CharROM.h"
#include "ColorRAM.h"


C64Memory::C64Memory()
{
    m_ptrRAM = new RAM();
    m_ptrBasicROM = new BasicROM();
    m_ptrKernalROM = new KernalROM();
    m_ptrCharROM = new CharROM();
    m_ptrColorRAM = new ColorRAM();
}
C64Memory::~C64Memory()
{
    delete m_ptrRAM;
    delete m_ptrBasicROM;
    delete m_ptrKernalROM;
    delete m_ptrCharROM;
    delete m_ptrColorRAM;
}

quint8 C64Memory::readRAM(const quint16 address) const
{
    return m_ptrRAM->read(address);
}
void C64Memory::writeRAM(const quint16 address, const quint8 value)
{
    m_ptrRAM->write(address, value);
}
