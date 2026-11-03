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
        return (m_portA & m_dataDirectionA) | (inputs & static_cast<quint8>(~m_dataDirectionA));
    }

    case 0x01:
    {
        const quint8 inputs = m_ptrPort ? m_ptrPort->portBInputs(portAOutput()) : m_portBInputs;
        return portBValue(inputs);
    }

    case 0x02:
        return m_dataDirectionA;

    case 0x03:
        return m_dataDirectionB;

    case 0x04:
        return static_cast<quint8>(m_timerACounter & 0x00FF);

    case 0x05:
        return static_cast<quint8>(m_timerACounter >> 8);

    case 0x06:
        return static_cast<quint8>(m_timerBCounter & 0x00FF);

    case 0x07:
        return static_cast<quint8>(m_timerBCounter >> 8);

    case 0x08:
    {
        const quint8 value = m_todReadLatched ? m_todReadLatch[0] : m_tod[0];

        // Reading tenths releases the read latch.
        m_todReadLatched = false;

        return value;
    }

    case 0x09:
        return m_todReadLatched ? m_todReadLatch[1] : m_tod[1];

    case 0x0A:
        return m_todReadLatched ? m_todReadLatch[2] : m_tod[2];

    case 0x0B:
        if (!m_todReadLatched)
        {
            for (int i = 0; i < 4; ++i)
                m_todReadLatch[i] = m_tod[i];

            m_todReadLatched = true;
        }

        return m_todReadLatch[3];

    case 0x0C:
        return m_serialDataRegister;

    case 0x0D:
    {
        quint8 value = m_interruptStatus;

        // Bit 7 is set if an enabled interrupt source is pending.
        if (m_interruptStatus & m_interruptMask)
            value |= 0x80;

        // Reading the ICR clears all pending interrupt status bits.
        m_interruptStatus = 0x00;

        return value;
    }

    case 0x0E:
        return m_controlRegisterA;

    case 0x0F:
        return m_controlRegisterB;

    default:
    {
        const quint8 reg = address & 0x0F;
        const quint16 mask = static_cast<quint16>(quint16(1) << reg);

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
}


void MOS6526::writeRegister(const quint8 address, const quint8 value)
{
    switch (address & 0x0F)
    {
    case 0x00:
        m_portA = value;

        if (m_ptrPort)
            m_ptrPort->portAOutputChanged(m_portA, m_dataDirectionA);

        return;

    case 0x01:
        m_portB = value;
        return;

    case 0x02:
        m_dataDirectionA = value;

        if (m_ptrPort)
            m_ptrPort->portAOutputChanged(m_portA, m_dataDirectionA);

        return;

    case 0x03:
        m_dataDirectionB = value;
        return;

    case 0x04:
        m_timerALatch = (m_timerALatch & 0xFF00) | value;
        break;

    case 0x05:
        m_timerALatch = (m_timerALatch & 0x00FF) | (static_cast<quint16>(value) << 8);

        // Writing the high byte loads the counter only while Timer A is stopped.
        if (!(m_controlRegisterA & 0x01))
            m_timerACounter = m_timerALatch;

        break;

    case 0x06:
        m_timerBLatch = (m_timerBLatch & 0xFF00) | value;
        break;

    case 0x07:
        m_timerBLatch = (m_timerBLatch & 0x00FF) | (static_cast<quint16>(value) << 8);

        // Writing the high byte loads the counter only while Timer B is stopped.
        if (!(m_controlRegisterB & 0x01))
            m_timerBCounter = m_timerBLatch;

        break;

    case 0x08:
        if (m_controlRegisterB & 0x80)
            m_todAlarm[0] = value & 0x0F;
        else
        {
            m_tod[0] = value & 0x0F;
            m_todStopped = false;
            m_todDividerCounter = 0;
        }
        return;

    case 0x09:
        if (m_controlRegisterB & 0x80)
            m_todAlarm[1] = value & 0x7F;
        else
            m_tod[1] = value & 0x7F;
        return;

    case 0x0A:
        if (m_controlRegisterB & 0x80)
            m_todAlarm[2] = value & 0x7F;
        else
            m_tod[2] = value & 0x7F;
        return;

    case 0x0B:
        if (m_controlRegisterB & 0x80)
            m_todAlarm[3] = value & 0x9F;
        else
        {
            m_tod[3] = value & 0x9F;
            m_todStopped = true;
            m_todDividerCounter = 0;
        }
        return;

    case 0x0C:
        m_serialDataRegister = value;

        if (m_controlRegisterA & 0x40)
            m_serialOutputPending = true;

        return;

    case 0x0D:
        // Bit 7 selects whether the specified interrupt mask bits are set or cleared.
        if (value & 0x80)
            m_interruptMask |= value & 0x1F;
        else
            m_interruptMask &= static_cast<quint8>(~(value & 0x1F));

        break;

    case 0x0E:
    {
        // CRA bit 0 rising edge starts Timer A and sets the toggle output HIGH.
        const bool startRisingEdge = !(m_controlRegisterA & 0x01) && (value & 0x01);

        if (startRisingEdge)
            m_timerAOutput = true;

        // CRA bit 4 is the Timer A Force Load strobe.
        if (value & 0x10)
            m_timerACounter = m_timerALatch;

        // Force Load is a strobe and is not stored.
        m_controlRegisterA = value & static_cast<quint8>(~0x10);
        break;
    }

    case 0x0F:
    {
        // CRB bit 0 rising edge starts Timer B and sets the toggle output HIGH.
        const bool startRisingEdge = !(m_controlRegisterB & 0x01) && (value & 0x01);

        if (startRisingEdge)
            m_timerBOutput = true;

        // CRB bit 4 is the Timer B Force Load strobe.
        if (value & 0x10)
            m_timerBCounter = m_timerBLatch;

        // Force Load is a strobe and is not stored.
        m_controlRegisterB = value & static_cast<quint8>(~0x10);
        break;
    }

    default:
    {
        const quint8 reg = address & 0x0F;
        const quint16 mask = static_cast<quint16>(quint16(1) << reg);

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
}


// Returns true when Timer A underflows.
bool MOS6526::clockTimerA()
{
    if (!(m_controlRegisterA & 0x01))
        return false;

    if (m_timerACounter != 0x0000)
    {
        --m_timerACounter;
        return false;
    }

    m_interruptStatus |= 0x01;
    m_timerACounter = m_timerALatch;

    // CRA bit 3 selects one-shot mode.
    if (m_controlRegisterA & 0x08)
        m_controlRegisterA &= static_cast<quint8>(~0x01);

    return true;
}


// Returns true when Timer B underflows.
bool MOS6526::clockTimerB()
{
    if (!(m_controlRegisterB & 0x01))
        return false;

    if (m_timerBCounter != 0x0000)
    {
        --m_timerBCounter;
        return false;
    }

    m_interruptStatus |= 0x02;
    m_timerBCounter = m_timerBLatch;

    // CRB bit 3 selects one-shot mode.
    if (m_controlRegisterB & 0x08)
        m_controlRegisterB &= static_cast<quint8>(~0x01);

    return true;
}


void MOS6526::timerAUnderflow()
{
    // The toggle flip-flop changes on every Timer A underflow.
    m_timerAOutput = !m_timerAOutput;

    // The pulse output remains active until the next PHI2 clock.
    m_timerAPulse = true;

    const quint8 source = (m_controlRegisterB >> 5) & 0x03;

    if (source == 0x02)
    {
        if (clockTimerB())
            timerBUnderflow();
    }
    else if (source == 0x03 && m_cnt)
    {
        if (clockTimerB())
            timerBUnderflow();
    }
}
void MOS6526::timerBUnderflow()
{
    // The toggle flip-flop changes on every Timer B underflow.
    m_timerBOutput = !m_timerBOutput;

    // The pulse output remains active until the next PHI2 clock.
    m_timerBPulse = true;
}

void MOS6526::clock()
{
    // Timer output pulses last for one PHI2 cycle.
    m_timerAPulse = false;
    m_timerBPulse = false;

    bool timerAUnderflowed = false;

    // Timer A source: Φ2 when CRA bit 5 is clear.
    if (!(m_controlRegisterA & 0x20))
        timerAUnderflowed = clockTimerA();

    // Timer B source: Φ2 when CRB bits 6-5 are 00.
    if (((m_controlRegisterB >> 5) & 0x03) == 0x00)
    {
        if (clockTimerB())
            timerBUnderflow();
    }

    if (timerAUnderflowed)
    {
        timerAUnderflow();
        serialPortOutputTimerAUnderflow();
    }
}


void MOS6526::setCNT(const bool level)
{
    const bool risingEdge = !m_cnt && level;
    m_cnt = level;

    if (!risingEdge)
        return;

    //
    // Serial input mode:
    // Shift one bit from SP into the serial data register.
    //
    if (!(m_controlRegisterA & 0x40))
    {
        m_serialDataRegister = static_cast<quint8>((m_serialDataRegister << 1) | (m_serialPortInput ? 0x01 : 0x00));

        ++m_serialBitCount;

        //
        // A complete byte has been received.
        //
        if (m_serialBitCount >= 8)
        {
            m_serialBitCount = 0;
            m_interruptStatus |= 0x08;
        }
    }

    bool timerAUnderflowed = false;

    // CRA bit 5 selects CNT as Timer A's clock source.
    if (m_controlRegisterA & 0x20)
        timerAUnderflowed = clockTimerA();

    // CRB bits 6-5 = 01 select CNT as Timer B's clock source.
    if (((m_controlRegisterB >> 5) & 0x03) == 0x01)
    {
        if (clockTimerB())
            timerBUnderflow();
    }

    if (timerAUnderflowed)
        timerAUnderflow();
}


void MOS6526::initialize()
{
    reset();
}


void MOS6526::reset()
{
    m_portA = 0x00;
    m_dataDirectionA = 0x00;
    m_portAInputs = 0xFF;

    m_portB = 0x00;
    m_dataDirectionB = 0x00;
    m_portBInputs = 0xFF;

    m_timerALatch = 0x0000;
    m_timerACounter = 0x0000;
    m_timerBLatch = 0x0000;
    m_timerBCounter = 0x0000;

    m_controlRegisterA = 0x00;
    m_controlRegisterB = 0x00;

    m_interruptStatus = 0x00;
    m_interruptMask = 0x00;

    m_unimplementedReadReported = 0x0000;
    m_unimplementedWriteReported = 0x0000;

    m_cnt = true;
    m_serialPortInput = true;
    m_serialPortOutput = true;

    // Notify connected hardware about the reset port state.
    if (m_ptrPort)
        m_ptrPort->portAOutputChanged(m_portA, m_dataDirectionA);

    for (int i = 0; i < 4; ++i)
    {
        m_tod[i] = 0x00;
        m_todAlarm[i] = 0x00;
        m_todReadLatch[i] = 0x00;
    }

    m_todDividerCounter = 0;
    m_todStopped = false;
    m_todReadLatched = false;

    m_serialDataRegister = 0x00;
    m_serialBitCount = 0;

    m_serialShiftRegister = 0x00;
    m_serialClockOutput = true;
    m_serialOutputPending = false;
    m_serialOutputActive = false;
    m_serialOutputClockPhase = false;

    m_flag = true;
    m_timerAOutput = false;
    m_timerBOutput = false;

    m_timerAPulse = false;
    m_timerBPulse = false;
}


void MOS6526::setTODDivider(const quint8 divider)
{
    if (divider == 0)
        return;

    m_todDivider = divider;
    m_todDividerCounter = 0;
}


void MOS6526::clockTODPulse()
{
    if (m_todStopped)
        return;

    ++m_todDividerCounter;

    if (m_todDividerCounter < m_todDivider)
        return;

    m_todDividerCounter = 0;

    incrementTOD();
    checkTODAlarm();
}



void MOS6526::incrementTOD()
{
    // Tenths.
    ++m_tod[0];

    if (m_tod[0] <= 0x09)
        return;

    m_tod[0] = 0x00;

    // Convert BCD to decimal, increment, then convert back.
    int seconds = ((m_tod[1] >> 4) * 10) + (m_tod[1] & 0x0F);
    ++seconds;

    if (seconds >= 60)
    {
        seconds = 0;
        int minutes = ((m_tod[2] >> 4) * 10) + (m_tod[2] & 0x0F);
        ++minutes;

        if (minutes >= 60)
        {
            minutes = 0;

            quint8 hour = m_tod[3] & 0x1F;
            const bool pm = (m_tod[3] & 0x80) != 0;

            if (hour == 0x11)
                m_tod[3] = static_cast<quint8>(0x12 | (pm ? 0x00 : 0x80));
            else if (hour == 0x12)
                m_tod[3] = static_cast<quint8>(0x01 | (pm ? 0x80 : 0x00));
            else
            {
                int decimalHour = ((hour >> 4) * 10) + (hour & 0x0F);
                ++decimalHour;
                hour = static_cast<quint8>(((decimalHour / 10) << 4) | (decimalHour % 10));
                m_tod[3] = static_cast<quint8>(hour | (pm ? 0x80 : 0x00));
            }
        }

        m_tod[2] = static_cast<quint8>(((minutes / 10) << 4) | (minutes % 10));
    }

    m_tod[1] = static_cast<quint8>(((seconds / 10) << 4) | (seconds % 10));
}


void MOS6526::checkTODAlarm()
{
    for (int i = 0; i < 4; ++i)
    {
        if (m_tod[i] != m_todAlarm[i])
            return;
    }

    m_interruptStatus |= 0x04;
}

void MOS6526::serialPortOutputTimerAUnderflow()
{
    if (!(m_controlRegisterA & 0x40))
        return;

    // Serial output requires Timer A to run in continuous mode.
    if (!(m_controlRegisterA & 0x01) || (m_controlRegisterA & 0x08))
        return;

    //
    // Load a pending byte when no byte is currently being transmitted.
    //
    if (!m_serialOutputActive && m_serialOutputPending)
    {
        m_serialShiftRegister = m_serialDataRegister;
        m_serialBitCount = 0;
        m_serialOutputPending = false;
        m_serialOutputActive = true;
        m_serialOutputClockPhase = false;
        m_serialClockOutput = true;
    }

    if (!m_serialOutputActive)
    {
        m_serialClockOutput = true;
        return;
    }

    //
    // Timer A generates one CNT transition per underflow.
    //
    m_serialOutputClockPhase = !m_serialOutputClockPhase;

    if (m_serialOutputClockPhase)
    {
        // Falling CNT edge: output the current MSB.
        m_serialClockOutput = false;
        m_serialPortOutput = (m_serialShiftRegister & 0x80) != 0;

        m_serialShiftRegister = static_cast<quint8>(m_serialShiftRegister << 1);
        ++m_serialBitCount;

        if (m_serialBitCount >= 8)
        {
            m_interruptStatus |= 0x08;
            m_serialOutputActive = false;
        }

        return;
    }

    // Rising CNT edge: keep SP stable.
    m_serialClockOutput = true;
}

void MOS6526::setFLAG(const bool level)
{
    const bool fallingEdge = m_flag && !level;
    m_flag = level;

    if (fallingEdge)
        m_interruptStatus |= 0x10;
}

quint8 MOS6526::portBValue(const quint8 inputs) const
{
    quint8 value = (m_portB & m_dataDirectionB) | (inputs & static_cast<quint8>(~m_dataDirectionB));

    // Timer A output overrides DDRB bit 6 when PBON is enabled.
    if (m_controlRegisterA & 0x02)
    {
        const bool output = (m_controlRegisterA & 0x04) ? m_timerAOutput : m_timerAPulse;

        if (output)
            value |= 0x40;
        else
            value &= static_cast<quint8>(~0x40);
    }

    // Timer B output overrides DDRB bit 7 when PBON is enabled.
    if (m_controlRegisterB & 0x02)
    {
        const bool output = (m_controlRegisterB & 0x04) ? m_timerBOutput : m_timerBPulse;

        if (output)
            value |= 0x80;
        else
            value &= static_cast<quint8>(~0x80);
    }

    return value;
}
