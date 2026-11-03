#include "C64CIA2Port.h"

#include <QDebug>


C64CIA2Port::C64CIA2Port()
{
}

C64CIA2Port::~C64CIA2Port()
{
    setIECBus(nullptr);
    setUserPortBus(nullptr);
}

void C64CIA2Port::setIECBus(IECBus* ptrIECBus)
{
    if (m_ptrIECBus == ptrIECBus)
        return;

    if (m_ptrIECBus != nullptr)
        m_ptrIECBus->removeParticipant(m_participantId);

    m_ptrIECBus = ptrIECBus;
    m_participantId = 0;


    if (m_ptrIECBus != nullptr)
    {
        m_participantId = m_ptrIECBus->addParticipant();

        qDebug() << "CIA2 IEC participant:" << m_participantId;
    }
}

void C64CIA2Port::setUserPortBus(UserPortBus* ptrUserPortBus)
{
    if (m_ptrUserPortBus == ptrUserPortBus)
        return;

    if (m_ptrUserPortBus != nullptr)
        m_ptrUserPortBus->removeParticipant(m_userPortParticipantId);

    m_ptrUserPortBus = ptrUserPortBus;
    m_userPortParticipantId = 0;

    if (m_ptrUserPortBus != nullptr)
        m_userPortParticipantId = m_ptrUserPortBus->addParticipant();

    // Den aktuellen CIA-Ausgangszustand auf den neuen Bus übertragen.
    if (m_ptrUserPortBus != nullptr)
    {
        m_ptrUserPortBus->setLineLow(m_userPortParticipantId, UserPortBus::Line::SP, !m_serialDataOutput);
        m_ptrUserPortBus->setLineLow(m_userPortParticipantId, UserPortBus::Line::CNT, !m_serialClockOutput);
    }
}


quint8 C64CIA2Port::portAInputs(quint8 portBOutput) const
{
    Q_UNUSED(portBOutput);

    quint8 inputs = 0xFF;

    if (!m_ptrIECBus)
        return inputs;

    const bool clockHigh = m_ptrIECBus->lineHigh(IECBus::Line::CLK);
    const bool dataHigh = m_ptrIECBus->lineHigh(IECBus::Line::DATA);

    if (!clockHigh)
        inputs &= static_cast<quint8>(~0x40);

    if (!dataHigh)
        inputs &= static_cast<quint8>(~0x80);

    static quint8 previousInputs = 0x00;

    if (inputs != previousInputs)
    {
        previousInputs = inputs;
    }

    return inputs;
}

quint8 C64CIA2Port::portBInputs(quint8 portAOutput) const
{
    Q_UNUSED(portAOutput);

    return 0xFF;
}

void C64CIA2Port::portAOutputChanged(quint8 portA, quint8 dataDirectionA)
{
    if (!m_ptrIECBus)
        return;

    //
    // CIA2 Port A:
    //
    // PA4 = IEC CLK OUT
    // PA5 = IEC DATA OUT
    // PA3 = IEC ATN OUT
    //
    // Output signals are inverted:
    // CIA output high pulls the IEC line low.
    //
    // Pins configured as inputs release their IEC line.
    //
    const bool clockLow = (dataDirectionA & 0x10) && (portA & 0x10);
    const bool dataLow = (dataDirectionA & 0x20) && (portA & 0x20);
    const bool attentionLow = (dataDirectionA & 0x08) && (portA & 0x08);

    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::CLK, clockLow);
    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::DATA, dataLow);
    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::ATN, attentionLow);
}

void C64CIA2Port::serialPortOutputChanged(const bool serialData, const bool serialClock)
{
    if (m_serialDataOutput == serialData &&
        m_serialClockOutput == serialClock)
    {
        return;
    }

    m_serialDataOutput = serialData;
    m_serialClockOutput = serialClock;

    if (m_ptrUserPortBus != nullptr)
    {
        m_ptrUserPortBus->setLineLow(m_userPortParticipantId, UserPortBus::Line::SP, !m_serialDataOutput);
        m_ptrUserPortBus->setLineLow(m_userPortParticipantId, UserPortBus::Line::CNT, !m_serialClockOutput);
    }

    qDebug() << "CIA2 USER PORT SERIAL OUT:"
             << "SP:" << m_serialDataOutput
             << "CNT:" << m_serialClockOutput;
}
