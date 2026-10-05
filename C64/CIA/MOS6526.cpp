#include "MOS6526.h"

#include <QDebug>

#include "MOS6526Port.h"


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
    {
        const quint8 inputs = m_ptrPort ? m_ptrPort->portAInputs(portBOutput()) : m_portAInputs;
        return (m_portA & m_dataDirectionA) | (inputs & ~m_dataDirectionA);
    }

    case 0x01:
    {
        const quint8 inputs = m_ptrPort ? m_ptrPort->portBInputs(portAOutput()) : m_portBInputs;
        return (m_portB & m_dataDirectionB) | (inputs & ~m_dataDirectionB);
    }
    case 0x02:
        return m_dataDirectionA;
    case 0x03:
        return m_dataDirectionB;
    case 0x04:
        return m_timerACounter & 0x00FF;
    case 0x05:
        return m_timerACounter >> 8;
    case 0x0D:
    {
        quint8 value = m_interruptStatus;

        //
        // Bit 7 is set if an enabled interrupt source
        // is pending.
        //
        if (m_interruptStatus & m_interruptMask)
            value |= 0x80;

        //
        // Reading the ICR clears all pending
        // interrupt status bits.
        //
        m_interruptStatus = 0x00;

        return value;
    }
    case 0x0E:
        return m_controlRegisterA;
    case 0x0F:
        return m_controlRegisterB;
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
    case 0x04:
        m_timerALatch = (m_timerALatch & 0xFF00) | value;
        break;
    case 0x05:
        m_timerALatch = (m_timerALatch & 0x00FF) | (static_cast<quint16>(value) << 8);

        //
        // Writing the high byte loads the counter
        // only while Timer A is stopped.
        //
        if (!(m_controlRegisterA & 0x01))
            m_timerACounter = m_timerALatch;
        break;
    case 0x0D:
        //
        // Bit 7 selects whether the specified interrupt
        // mask bits are set or cleared.
        //
        if (value & 0x80)
            m_interruptMask |= value & 0x1F;
        else
            m_interruptMask &= ~(value & 0x1F);
        break;
    case 0x0E:
        //
        // Bit 4 is the Force Load strobe.
        //
        if (value & 0x10)
            m_timerACounter = m_timerALatch;

        //
        // Force Load is a strobe and is not stored.
        //
        m_controlRegisterA = value & ~0x10;
        break;
    case 0x0F:
        m_controlRegisterB = value & ~0x10;
        break;
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

void MOS6526::clock()
{
    //
    // Timer A counts system clock cycles while START is set.
    //
    if (m_controlRegisterA & 0x01)
    {
        if (m_timerACounter == 0x0000)
        {
            //
            // Timer A underflow.
            //
            m_interruptStatus |= 0x01;

            //
            // Reload Timer A from the latch.
            //
            m_timerACounter = m_timerALatch;

            //
            // In one-shot mode an underflow stops Timer A.
            //
            if (m_controlRegisterA & 0x08)
                m_controlRegisterA &= ~0x01;
        }
        else
        {
            --m_timerACounter;
        }
    }
}
