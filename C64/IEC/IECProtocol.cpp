#include "IECProtocol.h"

#include <QDebug>


IECProtocol::IECProtocol(IECBus* ptrBus, const quint8 deviceNumber, const IECBus::ParticipantId participantId)
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

    m_transmitData.clear();
    m_transmitIndex = 0;
    m_transmitBit = 0;
    m_sendPhase = SendPhase::WaitReady;

    resetSendDiagnostics();
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

    // Alle Listener bestätigen ATN zunächst
    // durch DATA Low.
    setLineLow(IECBus::Line::DATA, true);

    m_transmitData.clear();
    m_transmitIndex = 0;
    m_transmitBit = 0;
    m_sendPhase = SendPhase::WaitReady;

    resetSendDiagnostics();
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

bool IECProtocol::startSending(const QByteArray& data)
{
    if (m_ptrBus == nullptr || m_mode != Mode::Talk || data.isEmpty())
        return false;

    if (m_state == State::Send || !lineHigh(IECBus::Line::ATN))
        return false;

    m_transmitData = data;
    m_transmitIndex = 0;
    m_transmitBit = 0;

    releaseBus();

    m_state = State::Send;
    m_sendPhase = SendPhase::WaitTalkTurnaround;
    m_stateCycles = 0;

    resetSendDiagnostics();

    return true;
}

void IECProtocol::clockSend()
{
    const SendPhase previousPhase = m_sendPhase;

    // Fortschritt liegt vor, sobald sich Phase, Byte oder Bit ändern.
    if (m_sendPhase != m_lastDiagnosticPhase ||
        m_transmitIndex != m_lastDiagnosticIndex ||
        m_transmitBit != m_lastDiagnosticBit)
    {
        m_sendNoProgressCycles = 0;
        m_lastDiagnosticPhase = m_sendPhase;
        m_lastDiagnosticIndex = m_transmitIndex;
        m_lastDiagnosticBit = m_transmitBit;
    }
    else
    {
        if (m_sendNoProgressCycles < 1000000U)
            ++m_sendNoProgressCycles;

        if (m_sendNoProgressCycles == 1000000U)
        {
            quint8 value = 0;

            if (m_transmitIndex >= 0 && m_transmitIndex < m_transmitData.size())
                value = static_cast<quint8>(m_transmitData.at(m_transmitIndex));

            qDebug() << "IEC TX stalled:"
                     << "phase:" << static_cast<int>(m_sendPhase)
                     << "index:" << m_transmitIndex
                     << "value:" << value
                     << "bit:" << m_transmitBit
                     << "CLK:" << lineHigh(IECBus::Line::CLK)
                     << "DATA:" << lineHigh(IECBus::Line::DATA)
                     << "ATN:" << lineHigh(IECBus::Line::ATN);

            ++m_sendNoProgressCycles;
        }
    }

    switch (m_sendPhase)
    {
    case SendPhase::WaitReady:
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);

        if (m_stateCycles == 1)
        {
            qDebug() << "IEC TX WaitReady:"
                     << "index:" << m_transmitIndex
                     << "CLK:" << lineHigh(IECBus::Line::CLK)
                     << "DATA:" << lineHigh(IECBus::Line::DATA);
        }

        if (lineHigh(IECBus::Line::CLK) && lineHigh(IECBus::Line::DATA))
        {
            if (m_stateCycles >= 2)
                prepareTransmitByte();
        }
        else
        {
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitEOIStart:
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);

        if (m_stateCycles >= EOI_TIMEOUT_CYCLES)
        {
            m_sendPhase = SendPhase::WaitEOIAcknowledge;
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
            qWarning() << "IEC TX timeout waiting for EOI acknowledge:"
                       << "index:" << m_transmitIndex;

            finishSending();
        }
        break;

    case SendPhase::WaitEOIRelease:
        // Der C64 muss die EOI-Antwort mindestens 60 µs halten.
        // Erst nach Ablauf dieser Zeit und Freigabe von DATA
        // darf das erste Datenbit beginnen.
        if (lineHigh(IECBus::Line::DATA) &&
            m_stateCycles >= SEND_EOI_RESPONSE_HOLD_CYCLES)
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
        // Der Listener darf CLK vorübergehend auf Low halten.
        // Die Datenhaltezeit beginnt erst, wenn CLK tatsächlich High ist.
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
                setLineLow(IECBus::Line::DATA, false);
                m_sendPhase = SendPhase::WaitByteAcknowledge;

                qDebug() << "IEC TX waiting for byte ACK:"
                         << "index:" << m_transmitIndex
                         << "value:" << static_cast<quint8>(m_transmitData.at(m_transmitIndex))
                         << "CLK:" << lineHigh(IECBus::Line::CLK)
                         << "DATA:" << lineHigh(IECBus::Line::DATA)
                         << "ATN:" << lineHigh(IECBus::Line::ATN);
            }

            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitByteAcknowledge:
        if (!lineHigh(IECBus::Line::DATA))
        {
            qDebug() << "IEC TX byte ACK received:"
                     << "index:" << m_transmitIndex
                     << "cycles:" << m_stateCycles;

            m_sendPhase = SendPhase::WaitAcknowledgeHold;
            m_stateCycles = 0;
        }
        else if (m_stateCycles >= FRAME_HANDSHAKE_TIMEOUT_CYCLES)
        {
            qWarning() << "IEC TX timeout waiting for byte ACK:"
                       << "index:" << m_transmitIndex
                       << "value:" << static_cast<quint8>(m_transmitData.at(m_transmitIndex));

            finishSending();
        }
        break;

    case SendPhase::WaitAcknowledgeHold:
        // Ein externes Gerät als Talker muss das Byte-ACK
        // mindestens 60 µs berücksichtigen.
        // CLK bleibt währenddessen Low.
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
                m_sendPhase = SendPhase::WaitBetweenBytes;
                m_stateCycles = 0;
            }
        }
        break;

    case SendPhase::WaitBetweenBytes:
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);

        // T_BB beginnt erst nach Freigabe des ACKs.
        if (m_stateCycles >= BETWEEN_BYTES_CYCLES)
        {
            ++m_transmitIndex;
            m_transmitBit = 0;
            m_sendPhase = SendPhase::WaitReady;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkTurnaround:
        // Der C64 gibt CLK frei, während DATA noch Low ist.
        // DATA kann dabei weiterhin durch das Laufwerk selbst
        // auf Low gehalten werden.
        if (lineHigh(IECBus::Line::CLK))
        {
            m_sendPhase = SendPhase::WaitTalkAttentionRelease;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkAttentionRelease:
        // Dem C64 Zeit geben, DATA als Listener zu übernehmen,
        // bevor das Laufwerk seine eigene DATA-Bestätigung freigibt.
        if (m_stateCycles >= TALK_ATTENTION_RELEASE_CYCLES)
        {
            setLineLow(IECBus::Line::DATA, false);
            setLineLow(IECBus::Line::CLK, true);

            m_sendPhase = SendPhase::WaitTalkAcknowledgeHold;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkAcknowledgeHold:
        // Das Laufwerk hält CLK mindestens 80 µs auf Low.
        // Danach gibt es CLK frei, damit der C64 als Listener
        // seine Bereitschaft durch DATA High signalisieren kann.
        if (m_stateCycles >= TALK_ATTENTION_ACK_HOLD_CYCLES)
        {
            setLineLow(IECBus::Line::CLK, false);

            m_sendPhase = SendPhase::WaitTalkListenerReady;
            m_stateCycles = 0;
        }
        break;

    case SendPhase::WaitTalkListenerReady:
        // Erst nach der Freigabe von CLK kann der C64 DATA
        // freigeben. Anschließend startet die Datenübertragung.
        if (lineHigh(IECBus::Line::DATA))
        {
            prepareTransmitByte();
        }
        break;
    }
    if (m_sendPhase != previousPhase)
    {
        quint8 value = 0;

        if (m_transmitIndex >= 0 && m_transmitIndex < m_transmitData.size())
            value = static_cast<quint8>(m_transmitData.at(m_transmitIndex));

        qDebug() << "IEC TX phase:"
                 << "from:" << static_cast<int>(previousPhase)
                 << "to:" << static_cast<int>(m_sendPhase)
                 << "index:" << m_transmitIndex
                 << "value:" << Qt::hex << value
                 << "bit:" << m_transmitBit
                 << "CLK:" << lineHigh(IECBus::Line::CLK)
                 << "DATA:" << lineHigh(IECBus::Line::DATA)
                 << "ATN:" << lineHigh(IECBus::Line::ATN);
    }
}

void IECProtocol::prepareTransmitByte()
{
    m_transmitBit = 0;
    m_stateCycles = 0;

    const qsizetype lastIndex = m_transmitData.size() - 1;

    if (m_transmitIndex == lastIndex)
    {
        // EOI wird vor dem letzten Byte signalisiert.
        setLineLow(IECBus::Line::CLK, false);
        setLineLow(IECBus::Line::DATA, false);
        m_sendPhase = SendPhase::WaitEOIStart;
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

    resetSendDiagnostics();
}

void IECProtocol::resetSendDiagnostics()
{
    m_sendNoProgressCycles = 0;
    m_lastDiagnosticPhase = m_sendPhase;
    m_lastDiagnosticIndex = m_transmitIndex;
    m_lastDiagnosticBit = m_transmitBit;
}
