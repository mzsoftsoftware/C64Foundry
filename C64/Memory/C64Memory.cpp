#include "C64Memory.h"

#include "RAM.h"
#include "ROM.h"
#include "ColorRAM.h"


C64Memory::C64Memory()
{
    m_ptrRAM = new RAM();

    m_ptrBasicROM = new ROM(8192);
    m_ptrKernalROM = new ROM(8192);
    m_ptrCharROM = new ROM(4096);

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

bool C64Memory::loadBasicROM(const QByteArray& data)
{
    return m_ptrBasicROM->load(data);
}
bool C64Memory::loadBasicROM(const QString& fileName)
{
    return m_ptrBasicROM->load(fileName);
}
bool C64Memory::loadKernalROM(const QByteArray& data)
{
    return m_ptrKernalROM->load(data);
}
bool C64Memory::loadKernalROM(const QString& fileName)
{
    return m_ptrKernalROM->load(fileName);
}
bool C64Memory::loadCharacterROM(const QByteArray& data)
{
    return m_ptrCharROM->load(data);
}
bool C64Memory::loadCharacterROM(const QString& fileName)
{
    return m_ptrCharROM->load(fileName);
}

quint8 C64Memory::readBasicROM(const quint16 address) const
{
    return m_ptrBasicROM->read(address);
}
quint8 C64Memory::readKernalROM(const quint16 address) const
{
    return m_ptrKernalROM->read(address);
}
quint8 C64Memory::readCharacterROM(const quint16 address) const
{
    return m_ptrCharROM->read(address);
}

quint8 C64Memory::readColorRAM(const quint16 address) const
{
    return m_ptrColorRAM->read(address);
}
void C64Memory::writeColorRAM(const quint16 address, const quint8 value)
{
    m_ptrColorRAM->write(address, value);
}