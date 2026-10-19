
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
    m_acknowledgeByte = false;

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
    const bool attentionRising = !m_previousAttentionHigh && attentionHigh;
    const bool clockRising = !m_previousClockHigh && clockHigh;
    const bool clockFalling = m_previousClockHigh && !clockHigh;

    if (!lineHigh(IECBus::Line::RESET))
    {
        reset();
        return;
    }

    if (attentionFalling)
        processAttention();

    if (attentionRising && m_state != State::ReceiveBits && m_state != State::WaitByteEnd && m_state != State::Acknowledge && m_state != State::AcknowledgeEOI)
    {
        if (m_mode != Mode::Listen)
        {
            releaseBus();
            m_state = State::Idle;
        }
        else
        {
            setLineLow(IECBus::Line::DATA, true);
            m_state = State::WaitClockLow;
            m_stateCycles = 0;
        }
    }

    ++m_stateCycles;

    switch (m_state)
    {
    case State::Idle:
        break;

    case State::Attention:
        if (clockHigh)
        {
            qDebug() << "IECProtocol: CLK HIGH, releasing DATA";

            setLineLow(IECBus::Line::DATA, false);

            m_state = State::WaitClockHigh;
            m_stateCycles = 0;
        }
        break;

    case State::WaitClockLow:
        if (m_stateCycles == 1)
        {
            qDebug() << "IECProtocol: WaitClockLow"
                     << "CLK=" << clockHigh
                     << "DATA=" << lineHigh(IECBus::Line::DATA)
                     << "ATN=" << attentionHigh;
        }

        if (!clockHigh)
        {
            qDebug() << "IECProtocol: CLK LOW, releasing DATA";

            setLineLow(IECBus::Line::DATA, false);

            m_state = State::WaitClockHigh;
            m_stateCycles = 0;
        }
        break;

    case State::WaitClockHigh:
        if (clockFalling)
        {
            qDebug() << "IECProtocol: First CLK falling, starting byte";

            m_receivedByte = 0;
            m_receivedBits = 0;
            m_endOfInformation = false;

            m_state = State::ReceiveBits;
            m_stateCycles = 0;
        }
        break;

    case State::ReceiveBits:
        if (m_receivedBits == 0 && clockHigh && attentionHigh && m_mode == Mode::Listen && m_stateCycles >= EOI_TIMEOUT_CYCLES)
        {
            // EOI: Der Talker hält CLK High, statt mit
            // der nächsten Byteübertragung zu beginnen.
            setLineLow(IECBus::Line::DATA, true);

            m_endOfInformation = true;
            m_state = State::AcknowledgeEOI;
            m_stateCycles = 0;
            break;
        }

        if (clockFalling)
        {
            qDebug() << "IECProtocol: CLK falling"
                     << "bits=" << m_receivedBits
                     << "ATN=" << attentionHigh;
        }

        if (clockRising)
        {
            qDebug() << "IECProtocol: CLK rising"
                     << "bits=" << m_receivedBits
                     << "DATA=" << lineHigh(IECBus::Line::DATA)
                     << "ATN=" << attentionHigh;
        }

        if (clockFalling)
            m_stateCycles = 0;

        if (clockRising)
        {
            // IEC überträgt das niederwertigste Bit zuerst.
            // DATA High entspricht einem gesetzten Datenbit.
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
        // Nach dem achten Bit zieht der Talker CLK
        // wieder auf Low und gibt DATA frei.
        if (clockFalling)
        {
            m_acknowledgeByte = processReceivedByte(m_receivedByte, !attentionHigh);

            if (m_acknowledgeByte)
                setLineLow(IECBus::Line::DATA, true);

            m_state = State::Acknowledge;
            m_stateCycles = 0;
        }
        break;

    case State::Acknowledge:
        if (attentionRising || m_stateCycles >= BYTE_ACK_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);

            qDebug() << "IECProtocol: ACK finished"
                     << "cycles=" << m_stateCycles
                     << "CLK=" << clockHigh
                     << "ATN=" << attentionHigh;

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
        // TALK-Betrieb wird später implementiert.
        break;
    }

    m_previousAttentionHigh = attentionHigh;
    m_previousClockHigh = clockHigh;
}

void IECProtocol::processAttention()
{
    qDebug() << "IECProtocol: ATN transition"
             << "state=" << static_cast<int>(m_state)
             << "bits=" << m_receivedBits
             << "value=" << Qt::hex << m_receivedByte
             << "CLK=" << lineHigh(IECBus::Line::CLK)
             << "DATA=" << lineHigh(IECBus::Line::DATA);

    releaseBus();

    m_state = State::Attention;
    m_stateCycles = 0;

    m_receivedByte = 0;
    m_receivedBits = 0;
    m_endOfInformation = false;
    m_acknowledgeByte = false;

    // Alle IEC-Listener reagieren zunächst auf ATN.
    setLineLow(IECBus::Line::DATA, true);

    qDebug() << "IECProtocol: ATN detected, device" << m_deviceNumber;
}

bool IECProtocol::processReceivedByte(const quint8 value, const bool attention)
{
    if (attention)
    {
        qDebug().nospace() << "IECProtocol: command $" << Qt::hex << value;

        if (value == 0x3F)
        {
            const bool wasListening = m_mode == Mode::Listen;
            m_mode = Mode::None;
            return wasListening;
        }

        if (value == 0x5F)
        {
            const bool wasTalking = m_mode == Mode::Talk;
            m_mode = Mode::None;
            return wasTalking;
        }

        if ((value & 0xE0) == 0x20)
        {
            m_mode = ((value & 0x1F) == m_deviceNumber) ? Mode::Listen : Mode::None;

            if (m_mode == Mode::Listen)
                qDebug() << "IECProtocol: LISTEN device" << m_deviceNumber;

            return m_mode == Mode::Listen;
        }

        if ((value & 0xE0) == 0x40)
        {
            m_mode = ((value & 0x1F) == m_deviceNumber) ? Mode::Talk : Mode::None;
            return m_mode == Mode::Talk;
        }

        if (m_mode == Mode::None)
            return false;

        if ((value & 0xF0) == 0x60 || (value & 0xF0) == 0xE0 || (value & 0xF0) == 0xF0)
        {
            m_secondaryAddress = value & 0x0F;

            qDebug() << "IECProtocol: secondary address" << m_secondaryAddress;
            return true;
        }

        return true;
    }

    if (m_mode == Mode::Listen)
    {
        ReceivedByte receivedByte;
        receivedByte.value = value;
        receivedByte.secondaryAddress = m_secondaryAddress;
        receivedByte.eoi = m_endOfInformation;

        m_receivedBytes.enqueue(receivedByte);

        qDebug().nospace() << "IECProtocol: data $" << Qt::hex << value;

        return true;
    }

    return false;
}

void IECProtocol::beginNextByte()
{
    m_receivedByte = 0;
    m_receivedBits = 0;
    m_endOfInformation = false;
    m_acknowledgeByte = false;
    m_stateCycles = 0;

    if (lineHigh(IECBus::Line::ATN) && m_mode != Mode::Listen)
    {
        m_state = State::Idle;
        releaseBus();
        return;
    }

    // Listener ist für das nächste Byte zunächst nicht bereit.
    setLineLow(IECBus::Line::DATA, false);
    m_state = State::WaitClockHigh;
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
