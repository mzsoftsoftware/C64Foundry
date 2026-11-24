#include "IECProtocol.h"


IECProtocol::IECProtocol(IECBus* ptrBus)
    : m_ptrBus(ptrBus)
{
    if (m_ptrBus != nullptr)
        m_participantId = m_ptrBus->addParticipant();

    reset();
}


IECProtocol::~IECProtocol()
{
    if (m_ptrBus != nullptr && m_participantId != 0)
        m_ptrBus->removeParticipant(m_participantId);
}


IECProtocol::State IECProtocol::state() const
{
    return m_state;
}


IECProtocolEvent::Mode IECProtocol::mode() const
{
    return m_mode;
}


quint8 IECProtocol::selectedDeviceNumber() const
{
    return m_selectedDeviceNumber;
}


quint8 IECProtocol::secondaryAddress() const
{
    return m_secondaryAddress;
}


void IECProtocol::reset()
{
    releaseBus();

    m_state = State::Idle;
    m_mode = IECProtocolEvent::Mode::None;

    m_selectedDeviceNumber = 0;
    m_secondaryAddress = 0;

    m_receivedByte = 0;
    m_receivedBits = 0;

    m_stateCycles = 0;

    m_endOfInformation = false;
    m_attentionClockLowSeen = false;

    m_events.clear();

    m_responseData.clear();
    m_responsePending = false;

    m_transmitData.clear();
    m_transmitIndex = 0;
    m_transmitBit = 0;
    m_sendPhase = SendPhase::WaitReady;

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

    // Die Antwort eines TALK-Geräts wird erst nach Ende der
    // ATN-Phase über den Bus übertragen.
    if (m_state == State::WaitFirstClockLow &&
        attentionHigh &&
        m_mode == IECProtocolEvent::Mode::Talk &&
        m_responsePending)
    {
        beginSending();
    }

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
        if (attentionHigh &&
            m_mode == IECProtocolEvent::Mode::Listen &&
            clockHigh &&
            m_stateCycles >= EOI_TIMEOUT_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, true);

            m_endOfInformation = true;
            m_state = State::AcknowledgeEOI;
            m_stateCycles = 0;

            break;
        }

        // Erste fallende CLK-Flanke startet die Byteübertragung.
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

    case State::WaitNextClockHigh:
        if (clockHigh)
        {
            setLineLow(IECBus::Line::DATA, false);

            if (attentionHigh && m_mode == IECProtocolEvent::Mode::None)
            {
                releaseBus();
                m_state = State::Idle;
            }
            else
            {
                m_state = State::WaitFirstClockLow;
            }

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
        if (!clockHigh)
        {
            m_receivedByte = 0;
            m_receivedBits = 0;

            m_state = State::ReceiveBits;
            m_stateCycles = 0;
        }
        break;

    case State::Send:
        clockSend();
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

    // Alle Listener bestätigen ATN zunächst durch DATA Low.
    setLineLow(IECBus::Line::DATA, true);

    m_responseData.clear();
    m_responsePending = false;

    m_transmitData.clear();
    m_transmitIndex = 0;
    m_transmitBit = 0;
    m_sendPhase = SendPhase::WaitReady;
}


void IECProtocol::processReceivedByte(const quint8 value, const bool attention)
{
    if (attention)
    {
        if (value == 0x3F)
        {
            if (m_mode == IECProtocolEvent::Mode::Listen)
                addEvent(IECProtocolEvent::Type::EndOfTransmission);

            if (m_mode != IECProtocolEvent::Mode::None)
                addEvent(IECProtocolEvent::Type::DeviceDeselected);

            m_mode = IECProtocolEvent::Mode::None;
            m_selectedDeviceNumber = 0;
            m_responseData.clear();
            m_responsePending = false;

            return;
        }

        if (value == 0x5F)
        {
            if (m_mode != IECProtocolEvent::Mode::None)
                addEvent(IECProtocolEvent::Type::DeviceDeselected);

            m_mode = IECProtocolEvent::Mode::None;
            m_selectedDeviceNumber = 0;
            m_responseData.clear();
            m_responsePending = false;

            return;
        }

        if ((value & 0xE0) == 0x20 || (value & 0xE0) == 0x40)
        {
            if (m_mode != IECProtocolEvent::Mode::None)
                addEvent(IECProtocolEvent::Type::DeviceDeselected);

            m_selectedDeviceNumber = value & 0x1F;

            m_mode = ((value & 0xE0) == 0x20)
                         ? IECProtocolEvent::Mode::Listen
                         : IECProtocolEvent::Mode::Talk;

            m_secondaryAddress = 0;
            m_responseData.clear();
            m_responsePending = false;

            addEvent(IECProtocolEvent::Type::DeviceSelected);

            return;
        }

        if (m_mode != IECProtocolEvent::Mode::None &&
            ((value & 0xF0) == 0x60 ||
             (value & 0xF0) == 0xE0 ||
             (value & 0xF0) == 0xF0))
        {
            m_secondaryAddress = value & 0x0F;
            addEvent(IECProtocolEvent::Type::SecondaryAddressReceived);
        }

        return;
    }

    if (m_mode == IECProtocolEvent::Mode::Listen)
    {
        IECProtocolEvent event;
        event.type = IECProtocolEvent::Type::ByteReceived;
        event.deviceNumber = m_selectedDeviceNumber;
        event.mode = m_mode;
        event.secondaryAddress = m_secondaryAddress;
        event.value = value;
        event.eoi = m_endOfInformation;

        m_events.enqueue(event);
        m_endOfInformation = false;
    }
}


void IECProtocol::addEvent(const IECProtocolEvent::Type type)
{
    IECProtocolEvent event;
    event.type = type;
    event.deviceNumber = m_selectedDeviceNumber;
    event.mode = m_mode;
    event.secondaryAddress = m_secondaryAddress;

    m_events.enqueue(event);
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

    if (m_mode != IECProtocolEvent::Mode::Listen)
    {
        releaseBus();
        m_state = State::Idle;
        return;
    }

    setLineLow(IECBus::Line::DATA, true);
    m_state = State::WaitNextClockHigh;
}


void IECProtocol::releaseBus()
{
    setLineLow(IECBus::Line::CLK, false);
    setLineLow(IECBus::Line::DATA, false);
}


QQueue<IECProtocolEvent> IECProtocol::takeEvents()
{
    QQueue<IECProtocolEvent> events;
    events.swap(m_events);
    return events;
}


void IECProtocol::setResponse(const IECProtocolResponse& response)
{
    m_responseData.clear();
    m_responsePending = false;

    if (response.type != IECProtocolResponse::Type::SendData ||
        response.data.isEmpty() ||
        m_mode != IECProtocolEvent::Mode::Talk)
    {
        return;
    }

    m_responseData = response.data;
    m_responsePending = true;
}


bool IECProtocol::lineHigh(const IECBus::Line line) const
{
    if (m_ptrBus == nullptr)
        return true;

    return m_ptrBus->lineHigh(line);
}


void IECProtocol::setLineLow(const IECBus::Line line, const bool low)
{
    if (m_ptrBus == nullptr || m_participantId == 0)
        return;

    m_ptrBus->setLineLow(m_participantId, line, low);
}


void IECProtocol::beginSending()
{
    if (m_responseData.isEmpty())
        return;

    m_transmitData = m_responseData;
    m_responseData.clear();
    m_responsePending = false;

    m_transmitIndex = 0;
    m_transmitBit = 0;

    // Während des TALK-Turnarounds hält das Laufwerk DATA zunächst
    // weiterhin Low. CLK wird freigegeben, damit der C64 die
    // Übergabe durch seine CLK-Freigabe einleiten kann.
    setLineLow(IECBus::Line::DATA, true);
    setLineLow(IECBus::Line::CLK, false);

    m_state = State::Send;
    m_sendPhase = SendPhase::WaitTalkTurnaround;
    m_stateCycles = 0;
}


void IECProtocol::clockSend()
{
    switch (m_sendPhase)
    {
    case SendPhase::WaitReady:
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);

        if (lineHigh(IECBus::Line::CLK) && lineHigh(IECBus::Line::DATA))
        {
            if (m_stateCycles >= TALK_LISTENER_READY_DELAY_CYCLES)
                prepareTransmitByte();
        }
        else
        {
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitEOIAcknowledge:
        if (!lineHigh(IECBus::Line::DATA))
        {
            m_sendPhase = SendPhase::WaitEOIRelease;
            m_stateCycles = 0;
        }
        else if (m_stateCycles >= FRAME_HANDSHAKE_TIMEOUT_CYCLES)
        {
            finishSending();
        }
        break;

    case SendPhase::WaitEOIRelease:
        if (lineHigh(IECBus::Line::DATA))
        {
            m_transmitBit = 0;
            m_sendPhase = SendPhase::DriveBit;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::DriveBit:
    {
        const quint8 value = static_cast<quint8>(m_transmitData.at(m_transmitIndex));
        const bool bitHigh = (value & static_cast<quint8>(1U << m_transmitBit)) != 0;

        setLineLow(IECBus::Line::DATA, !bitHigh);
        setLineLow(IECBus::Line::CLK, true);

        m_sendPhase = SendPhase::WaitClockLow;
        m_stateCycles = 0;
        break;
    }

    case SendPhase::WaitClockLow:
        if (m_stateCycles >= SEND_BIT_SETUP_CYCLES)
        {
            setLineLow(IECBus::Line::CLK, false);
            m_sendPhase = SendPhase::WaitClockHigh;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitClockHigh:
        if (!lineHigh(IECBus::Line::CLK))
        {
            m_stateCycles = 0;
            break;
        }

        if (m_stateCycles >= SEND_DATA_VALID_CYCLES)
        {
            setLineLow(IECBus::Line::CLK, true);

            if (m_transmitBit < 7)
            {
                ++m_transmitBit;
                m_sendPhase = SendPhase::DriveBit;
            }
            else
            {
                m_sendPhase = SendPhase::WaitFinalBitDataHold;
            }

            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitFinalBitDataHold:
        if (m_stateCycles >= SEND_FINAL_BIT_DATA_HOLD_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);

            m_sendPhase = SendPhase::WaitByteAcknowledge;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitByteAcknowledge:
        if (!lineHigh(IECBus::Line::DATA))
        {
            m_sendPhase = SendPhase::WaitAcknowledgeHold;
            m_stateCycles = 0;
        }
        else if (m_stateCycles >= FRAME_HANDSHAKE_TIMEOUT_CYCLES)
        {
            finishSending();
        }
        break;

    case SendPhase::WaitAcknowledgeHold:
        if (m_stateCycles >= SEND_BYTE_ACK_HOLD_CYCLES)
        {
            setLineLow(IECBus::Line::CLK, false);

            m_sendPhase = SendPhase::WaitAcknowledgeRelease;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitAcknowledgeRelease:
        if (lineHigh(IECBus::Line::DATA))
        {
            if (m_transmitIndex + 1 >= m_transmitData.size())
            {
                finishSending();
            }
            else
            {
                ++m_transmitIndex;
                m_transmitBit = 0;

                if (m_transmitIndex == m_transmitData.size() - 1)
                {
                    prepareTransmitByte();
                }
                else
                {
                    m_sendPhase = SendPhase::WaitBetweenBytes;
                    m_stateCycles = 0;
                }
            }
        }
        break;

    case SendPhase::WaitBetweenBytes:
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);

        if (m_stateCycles >= BETWEEN_BYTES_CYCLES)
        {
            m_transmitBit = 0;
            m_sendPhase = SendPhase::WaitReady;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkTurnaround:
        if (lineHigh(IECBus::Line::CLK))
        {
            setLineLow(IECBus::Line::CLK, true);
            setLineLow(IECBus::Line::DATA, false);

            m_sendPhase = SendPhase::WaitTalkAcknowledgeHold;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkAcknowledgeHold:
        if (m_stateCycles >= TALK_ATTENTION_ACK_HOLD_CYCLES)
        {
            setLineLow(IECBus::Line::CLK, false);

            m_sendPhase = SendPhase::WaitTalkListenerReady;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkListenerReady:
        if (lineHigh(IECBus::Line::DATA))
        {
            m_sendPhase = SendPhase::WaitInitialDataStart;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitInitialDataStart:
        if (m_stateCycles >= TALK_LISTENER_READY_DELAY_CYCLES)
            prepareTransmitByte();
        break;
    }
}


void IECProtocol::prepareTransmitByte()
{
    m_transmitBit = 0;
    m_stateCycles = 0;

    const qsizetype lastIndex = m_transmitData.size() - 1;

    if (m_transmitIndex == lastIndex)
    {
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);

        m_sendPhase = SendPhase::WaitEOIAcknowledge;
    }
    else
    {
        m_sendPhase = SendPhase::DriveBit;
    }
}


void IECProtocol::finishSending()
{
    releaseBus();

    m_transmitData.clear();
    m_transmitIndex = 0;
    m_transmitBit = 0;
    m_stateCycles = 0;

    m_state = State::Idle;
}
