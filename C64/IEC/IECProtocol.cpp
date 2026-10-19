#include "IECProtocol.h"

#include <QDebug>


IECProtocol::IECProtocol(const quint8 deviceNumber, IECBus* ptrBus, const IECBus::ParticipantId participantId)
    : m_deviceNumber(deviceNumber)
    , m_ptrBus(ptrBus)
    , m_participantId(participantId)
{
    reset();
}

void IECProtocol::reset()
{
    releaseBus();

    m_state = State::Idle;
    m_mode = Mode::None;

    m_secondaryAddress = 0;
    m_receivedByte = 0;
    m_receivedBits = 0;
    m_stateCycles = 0;
    m_endOfInformation = false;

    m_receivedBytes.clear();

    m_previousAttentionHigh = lineHigh(IECBus::Line::ATN);
    m_previousClockHigh = lineHigh(IECBus::Line::CLK);
}

void IECProtocol::clock()
{
    if (m_ptrBus == nullptr)
        return;

    const bool attentionHigh = lineHigh(IECBus::Line::ATN);
    const bool clockHigh = lineHigh(IECBus::Line::CLK);

    const bool attentionFalling = m_previousAttentionHigh && !attentionHigh;
    const bool clockRising = !m_previousClockHigh && clockHigh;
    const bool clockFalling = m_previousClockHigh && !clockHigh;

    if (!lineHigh(IECBus::Line::RESET))
    {
        reset();
        return;
    }

    if (attentionFalling)
        processAttention();

    ++m_stateCycles;

    switch (m_state)
    {
    case State::Idle:
        break;

    case State::Attention:
        m_state = State::WaitClockLow;
        m_stateCycles = 0;
        break;

    case State::WaitClockLow:
        if (!clockHigh)
        {
            qDebug() << "IECProtocol: CLK LOW, releasing DATA";

            setLineLow(IECBus::Line::DATA, false);
            m_state = State::WaitClockHigh;
            m_stateCycles = 0;
        }
        break;

    case State::WaitClockHigh:
        if (clockHigh)
        {
            m_stateCycles = 0;
            m_state = State::ReceiveBits;
            m_receivedByte = 0;
            m_receivedBits = 0;
            m_endOfInformation = false;
        }
        break;

    case State::ReceiveBits:
        if (m_receivedBits == 0 && clockHigh && attentionHigh && m_mode == Mode::Listen && m_stateCycles >= EOI_TIMEOUT_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, true);
            m_endOfInformation = true;
            m_state = State::AcknowledgeEOI;
            m_stateCycles = 0;
            break;
        }

        if (clockFalling)
        {
            m_stateCycles = 0;
        }

        if (clockRising)
        {
            if (lineHigh(IECBus::Line::DATA))
                m_receivedByte |= static_cast<quint8>(1U << m_receivedBits);

            ++m_receivedBits;

            if (m_receivedBits == 8)
            {
                m_state = State::WaitByteEnd;
                m_stateCycles = 0;
            }
        }
        break;

    case State::WaitByteEnd:
        if (clockFalling)
        {
            processReceivedByte(m_receivedByte, !attentionHigh);

            setLineLow(IECBus::Line::DATA, true);

            m_state = State::Acknowledge;
            m_stateCycles = 0;
        }
        break;

    case State::Acknowledge:
        if (m_stateCycles >= BYTE_ACK_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);
            beginNextByte();
        }
        break;

    case State::AcknowledgeEOI:
        if (m_stateCycles >= EOI_ACK_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);

            m_state = State::ReceiveBits;
            m_stateCycles = 0;
        }
        break;

    case State::Send:
        break;
    }

    m_previousAttentionHigh = attentionHigh;
    m_previousClockHigh = clockHigh;
}

void IECProtocol::processAttention()
{
    qDebug() << "IECProtocol: ATN detected, device" << m_deviceNumber;

    releaseBus();

    m_state = State::Attention;
    m_stateCycles = 0;

    m_receivedByte = 0;
    m_receivedBits = 0;
    m_endOfInformation = false;

    // ATN wird von allen IEC-Geräten bestätigt.
    setLineLow(IECBus::Line::DATA, true);
}

void IECProtocol::processReceivedByte(const quint8 value, const bool attention)
{
    if (attention)
    {
        if (value == 0x3F)
        {
            m_mode = Mode::None;
            return;
        }

        if (value == 0x5F)
        {
            m_mode = Mode::None;
            return;
        }

        if ((value & 0xE0) == 0x20)
        {
            m_mode = ((value & 0x1F) == m_deviceNumber) ? Mode::Listen : Mode::None;
            return;
        }

        if ((value & 0xE0) == 0x40)
        {
            m_mode = ((value & 0x1F) == m_deviceNumber) ? Mode::Talk : Mode::None;
            return;
        }

        if ((value & 0xE0) == 0x60 && m_mode != Mode::None)
        {
            m_secondaryAddress = value & 0x0F;
            return;
        }

        if ((value & 0xF0) == 0xE0 && m_mode != Mode::None)
        {
            m_secondaryAddress = value & 0x0F;
            return;
        }

        if ((value & 0xF0) == 0xF0 && m_mode != Mode::None)
        {
            m_secondaryAddress = value & 0x0F;
            return;
        }

        return;
    }

    if (m_mode == Mode::Listen)
    {
        ReceivedByte receivedByte;
        receivedByte.value = value;
        receivedByte.secondaryAddress = m_secondaryAddress;
        receivedByte.eoi = m_endOfInformation;

        m_receivedBytes.enqueue(receivedByte);
    }
}

void IECProtocol::beginNextByte()
{
    m_receivedByte = 0;
    m_receivedBits = 0;
    m_endOfInformation = false;
    m_stateCycles = 0;

    if (lineHigh(IECBus::Line::ATN) && m_mode != Mode::Listen)
    {
        m_state = State::Idle;
        releaseBus();
    }
    else
    {
        m_state = State::WaitClockHigh;
    }
}

void IECProtocol::releaseBus()
{
    setLineLow(IECBus::Line::CLK, false);
    setLineLow(IECBus::Line::DATA, false);
}

bool IECProtocol::hasReceivedByte() const
{
    return !m_receivedBytes.isEmpty();
}

IECProtocol::ReceivedByte IECProtocol::takeReceivedByte()
{
    if (m_receivedBytes.isEmpty())
        return ReceivedByte();

    return m_receivedBytes.dequeue();
}

bool IECProtocol::lineHigh(const IECBus::Line line) const
{
    if (m_ptrBus == nullptr)
        return true;

    return m_ptrBus->lineHigh(line);
}

void IECProtocol::setLineLow(const IECBus::Line line, const bool low)
{
    if (m_ptrBus == nullptr)
        return;

    m_ptrBus->setLineLow(m_participantId, line, low);
}
