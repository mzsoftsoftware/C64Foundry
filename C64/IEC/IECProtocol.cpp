
#include "IECProtocol.h"


IECProtocol::IECProtocol(const quint8 deviceNumber, IECBus* ptrBus, const IECBus::ParticipantId participantId)
    : m_deviceNumber(deviceNumber)
    , m_ptrBus(ptrBus)
    , m_participantId(participantId)
{
    reset();
}

void IECProtocol::reset()
{
    m_state = State::Idle;
    m_mode = Mode::None;

    m_secondaryAddress = 0;
    m_receivedByte = 0;
    m_receivedBits = 0;

    m_previousAttentionHigh = lineHigh(IECBus::Line::ATN);
    m_previousClockHigh = lineHigh(IECBus::Line::CLK);

    setLineLow(IECBus::Line::CLK, false);
    setLineLow(IECBus::Line::DATA, false);
}

void IECProtocol::clock()
{
    if (m_ptrBus == nullptr)
        return;

    const bool attentionHigh = lineHigh(IECBus::Line::ATN);
    const bool clockHigh = lineHigh(IECBus::Line::CLK);

    if (attentionHigh != m_previousAttentionHigh)
    {
        if (!attentionHigh)
            processAttention();
        else if (m_state == State::Attention)
            m_state = State::Idle;
    }

    m_previousAttentionHigh = attentionHigh;
    m_previousClockHigh = clockHigh;
}

void IECProtocol::processAttention()
{
    m_state = State::Attention;

    m_receivedByte = 0;
    m_receivedBits = 0;
}

void IECProtocol::processReceivedByte(const quint8 value, const bool attention)
{
    if (!attention)
        return;

    if (value == 0x3F)
    {
        m_mode = Mode::None;
        m_state = State::Idle;
        return;
    }

    if (value == 0x5F)
    {
        m_mode = Mode::None;
        m_state = State::Idle;
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
        m_secondaryAddress = value & 0x1F;
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
