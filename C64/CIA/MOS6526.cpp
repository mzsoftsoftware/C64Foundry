#include "MOS6526.h"

#include <QDebug>


MOS6526::MOS6526()
{
}
MOS6526::~MOS6526()
{
}


quint8 MOS6526::readRegister(const quint8 address)
{
    switch (address & 0x0F)
    {
    case 0x00:
        return (m_portA & m_dataDirectionA) | (m_portAInputs & ~m_dataDirectionA);
    case 0x01:
        return (m_portB & m_dataDirectionB) | (m_portBInputs & ~m_dataDirectionB);
    case 0x02:
        return m_dataDirectionA;
    case 0x03:
        return m_dataDirectionB;
    default:
        const quint8 reg = address & 0x0F;
        const quint16 mask = quint16(1) << reg;

        if (!(m_unimplementedReadReported & mask))
        {
            qDebug().nospace()
            << "MOS6526: read access to unimplemented register $"
            << Qt::hex << reg
            << " -> $FF";

            m_unimplementedReadReported |= mask;
        }

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
    case 0x01:
        m_portB = value;
        return;
    case 0x02:
        m_dataDirectionA = value;
        return;
    case 0x03:
        m_dataDirectionB = value;
        return;
    default:
        const quint8 reg = address & 0x0F;
        const quint16 mask = quint16(1) << reg;

        if (!(m_unimplementedWriteReported & mask))
        {
            qDebug().nospace()
            << "MOS6526: write access to unimplemented register $"
            << Qt::hex << reg
            << " <- $" << value;

            m_unimplementedWriteReported |= mask;
        }

        return;
    }
}
