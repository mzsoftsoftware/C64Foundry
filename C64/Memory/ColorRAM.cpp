#include "ColorRAM.h"


ColorRAM::ColorRAM()
{
    m_data = QByteArray(1024, 0);
}
ColorRAM::~ColorRAM()
{
}

quint8 ColorRAM::read(const quint16 address) const
{
    return static_cast<quint8>(m_data.at(address)) & 0x0F;
}

void ColorRAM::write(const quint16 address, const quint8 value)
{
    m_data[address] = static_cast<char>(value & 0x0F);
}