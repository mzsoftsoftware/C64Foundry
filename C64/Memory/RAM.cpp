#include "RAM.h"

#include <cstring>


RAM::RAM()
{
    memset(m_data, 0, sizeof(m_data));
}
RAM::~RAM()
{
}

quint8 RAM::read(const quint16 address) const
{
    return m_data[address];
}

void RAM::write(const quint16 address, const quint8 value)
{
    m_data[address] = value;
}
