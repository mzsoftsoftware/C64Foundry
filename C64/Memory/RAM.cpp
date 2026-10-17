#include "RAM.h"

#include <cstring>


RAM::RAM()
{
}
RAM::~RAM()
{
}

void RAM::initialize()
{
    memset(m_data, 0, sizeof(m_data));
}

quint8 RAM::read(const quint16 address) const
{
    return m_data[address];
}

void RAM::write(const quint16 address, const quint8 value)
{
    m_data[address] = value;
}
