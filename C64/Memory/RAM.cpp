#include "RAM.h"


RAM::RAM()
{
    m_data = QByteArray(65536,0);
}
RAM::~RAM()
{
}

quint8 RAM::read(const quint16 address) const
{
    return static_cast<quint8>(m_data.at(address));
}

void RAM::write(const quint16 address, const quint8 value)
{
    m_data[address] = static_cast<char>(value);
}
