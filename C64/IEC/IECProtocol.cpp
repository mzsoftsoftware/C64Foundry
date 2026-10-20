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
    releaseBus();

    m_state = State::Idle;
    m_mode = Mode::None;

    m_secondaryAddress = 0;
    m_receivedByte = 0;
    m_receivedBits = 0;
    m_listenEnded = false;
    m_stateCycles = 0;

    m_endOfInformation = false;
    m_attentionClockLowSeen = false;

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
        // Während ATN Low ist, bestätigt der Listener
        // die Attention-Phase zunächst mit DATA Low.
        //
        // CLK muss während dieser ATN-Phase zunächst
        // Low gewesen sein und anschließend High werden.
        // Dadurch geben wir DATA nicht sofort frei,
        // wenn CLK beim ATN-Einstieg bereits High ist.

        if (!clockHigh)
            m_attentionClockLowSeen = true;

        if (m_attentionClockLowSeen && clockHigh)
        {
            setLineLow(IECBus::Line::DATA, false);
            m_state = State::WaitFirstClockLow;
            m_stateCycles = 0;
        }
        break;

    case State::WaitClockHigh:
        if (clockHigh)
        {
            m_state = State::WaitFirstClockLow;
            m_stateCycles = 0;
        }
        break;

    case State::WaitFirstClockLow:
        // EOI wird vor dem ersten Datenbit erkannt.
        // EOI gilt nur während der Datenphase.
        if (attentionHigh && m_mode == Mode::Listen && clockHigh && m_stateCycles >= EOI_TIMEOUT_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, true);

            m_endOfInformation = true;
            m_state = State::AcknowledgeEOI;
            m_stateCycles = 0;

            break;
        }

        // Erste fallende CLK-Flanke startet die
        // eigentliche Byteübertragung.
        if (clockFalling)
        {
            m_receivedByte = 0;
            m_receivedBits = 0;

            m_state = State::ReceiveBits;
            m_stateCycles = 0;
        }
        break;

    case State::ReceiveBits:
        // IEC überträgt das niederwertigste Bit zuerst.
        // DATA wird bei steigender CLK-Flanke gelesen.
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
        // Nach dem achten Bit zieht der Talker CLK Low.
        // Anschließend bestätigt der Listener das Byte.
        if (clockFalling)
        {
            processReceivedByte(m_receivedByte, !attentionHigh);

            setLineLow(IECBus::Line::DATA, true);

            m_state = State::Acknowledge;
            m_stateCycles = 0;
        }
        break;

    case State::Acknowledge:
        // ACK unabhängig vom ATN-Pegel abschließen.
        if (m_stateCycles >= BYTE_ACK_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);
            beginNextByte();
        }
        break;

    case State::WaitNextClockHigh:
        // Nach dem Byte-ACK hält der Listener
        // DATA zunächst Low.
        //
        // Sobald der Talker CLK freigibt, wird
        // DATA freigegeben und das nächste Byte erwartet.
        if (clockHigh)
        {
            setLineLow(IECBus::Line::DATA, false);

            m_state = State::WaitFirstClockLow;
            m_stateCycles = 0;
        }
        break;

    case State::AcknowledgeEOI:
        if (m_stateCycles >= EOI_ACK_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);

            m_state = State::WaitEOIRelease;
            m_stateCycles = 0;
        }
        break;

    case State::WaitEOIRelease:
        // Nach der EOI-Bestätigung warten wir auf
        // den Beginn des letzten Bytes.
        if (!clockHigh)
        {
            m_receivedByte = 0;
            m_receivedBits = 0;

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
    releaseBus();

    m_state = State::Attention;
    m_stateCycles = 0;

    m_receivedByte = 0;
    m_receivedBits = 0;
    m_endOfInformation = false;
    m_attentionClockLowSeen = false;

    // Alle Listener bestätigen ATN zunächst
    // durch DATA Low.
    setLineLow(IECBus::Line::DATA, true);
}

void IECProtocol::processReceivedByte(const quint8 value, const bool attention)
{
    if (attention)
    {
        if (value == 0x3F)
        {
            if (m_mode == Mode::Listen)
                m_listenEnded = true;

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

        if (m_mode != Mode::None && ((value & 0xF0) == 0x60 || (value & 0xF0) == 0xE0 || (value & 0xF0) == 0xF0))
        {
            m_secondaryAddress = value & 0x0F;
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

    // Während ATN Low ist, können weitere Kommandos folgen.
    if (!lineHigh(IECBus::Line::ATN))
    {
        setLineLow(IECBus::Line::DATA, true);

        m_state = State::WaitNextClockHigh;
        return;
    }

    // Nach UNLISTEN ist das Gerät nicht mehr adressiert.
    if (m_mode != Mode::Listen)
    {
        releaseBus();
        m_state = State::Idle;
        return;
    }

    // Adressierter Listener wartet auf das nächste Byte.
    setLineLow(IECBus::Line::DATA, true);

    m_state = State::WaitNextClockHigh;
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

bool IECProtocol::takeListenEnded()
{
    const bool ended = m_listenEnded;
    m_listenEnded = false;
    return ended;
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
