#include "MOS6526.h"


MOS6526::MOS6526()
{
}
MOS6526::~MOS6526()
{
}


quint8 MOS6526::readRegister(const quint8 address) const
{
    switch (address & 0x0F)
    {
    case 0x00:
        return m_portA;

    default:
        return 0xFF;
    }
}

void MOS6526::writeRegister(const quint8 address, const quint8 value)
{
    switch (address & 0x0F)
    {
    case 0x00:
        m_portA = value;
        return;

    default:
        return;
    }
}
