#include "ColorRAM.h"

#include <cstring>


ColorRAM::ColorRAM()
{
}
ColorRAM::~ColorRAM()
{
}

void ColorRAM::initialize()
{
    for (quint16 address = 0; address < 1024; ++address)
        m_data[address] = 0x00;
}

quint8 ColorRAM::read(const quint16 address) const
{
    return m_data[address] & 0x0F;
}

void ColorRAM::write(const quint16 address, const quint8 value)
{
    m_data[address] = value & 0x0F;
}