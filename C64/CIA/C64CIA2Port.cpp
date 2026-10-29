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
        m_participantId = m_ptrIECBus->addParticipant();
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
    // PA6 = IEC ATN OUT
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
