#include "C64CIA2Port.h"

#include <QDebug>


C64CIA2Port::C64CIA2Port()
{
}

C64CIA2Port::~C64CIA2Port()
{
    setIECBus(nullptr);
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

    const bool attentionHigh = m_ptrIECBus->lineHigh(IECBus::Line::ATN);
    //
    // Log CIA2 Port A reads during the IEC data phase.
    // Reset the counter while ATN is active.
    //
    if (!attentionHigh)
    {
        m_portAReadLogCount = 0;
    }
    else if (m_portAReadLogCount < 256)
    {
        ++m_portAReadLogCount;

        qDebug() << "CIA2 IEC Port A read:"
                 << "read:" << m_portAReadLogCount
                 << "CLK high:" << clockHigh
                 << "DATA high:" << dataHigh
                 << "ATN high:" << attentionHigh
                 << "inputs:" << Qt::hex << inputs;
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

    const bool busClockHigh = m_ptrIECBus->lineHigh(IECBus::Line::CLK);
    const bool busAttentionHigh = m_ptrIECBus->lineHigh(IECBus::Line::ATN);

    //
    // Diagnose CIA2 output changes only during the byte-ACK window:
    // CLK is held LOW by the talker and ATN is released.
    //
    if (!busClockHigh && busAttentionHigh)
    {
        qDebug() << "CIA2 IEC ACK window:"
                 << "PA:" << Qt::hex << portA
                 << "DDRA:" << Qt::hex << dataDirectionA
                 << "CLK low:" << clockLow
                 << "DATA low:" << dataLow;
    }

    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::CLK, clockLow);
    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::DATA, dataLow);
    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::ATN, attentionLow);
}
