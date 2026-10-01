#include "ColorRAM.h"

#include <cstring>


ColorRAM::ColorRAM()
{
    std::memset(m_data, 0, sizeof(m_data));
}
ColorRAM::~ColorRAM()
{
}

quint8 ColorRAM::read(const quint16 address) const
{
    return m_data[address] & 0x0F;
}

void ColorRAM::write(const quint16 address, const quint8 value)
{
    m_data[address] = value & 0x0F;
}