#include "C64CIA2Port.h"


C64CIA2Port::C64CIA2Port(IECBus* ptrBus)
    : m_ptrBus(ptrBus)
{
    if (m_ptrBus)
        m_participantId = m_ptrBus->addParticipant();
}

C64CIA2Port::~C64CIA2Port()
{
    if (m_ptrBus)
        m_ptrBus->removeParticipant(m_participantId);
}

quint8 C64CIA2Port::portAInputs(quint8 portBOutput) const
{
    Q_UNUSED(portBOutput);

    quint8 inputs = 0xFF;

    if (!m_ptrBus)
        return inputs;

    //
    // PA2 = IEC CLK IN
    // PA3 = IEC DATA IN
    //
    if (!m_ptrBus->lineHigh(IECBus::Line::CLK))
        inputs &= static_cast<quint8>(~0x04);

    if (!m_ptrBus->lineHigh(IECBus::Line::DATA))
        inputs &= static_cast<quint8>(~0x08);

    return inputs;
}

quint8 C64CIA2Port::portBInputs(quint8 portAOutput) const
{
    Q_UNUSED(portAOutput);

    return 0xFF;
}

void C64CIA2Port::portAOutputChanged(quint8 portA, quint8 dataDirectionA)
{
    if (!m_ptrBus)
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
    const bool attentionLow = (dataDirectionA & 0x40) && (portA & 0x40);

    m_ptrBus->setLineLow(m_participantId, IECBus::Line::CLK, clockLow);
    m_ptrBus->setLineLow(m_participantId, IECBus::Line::DATA, dataLow);
    m_ptrBus->setLineLow(m_participantId, IECBus::Line::ATN, attentionLow);
}
