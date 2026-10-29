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

    //
    // PA2 = IEC CLK IN
    // PA3 = IEC DATA IN
    //
    if (!m_ptrIECBus->lineHigh(IECBus::Line::CLK))
        inputs &= static_cast<quint8>(~0x40);

    if (!m_ptrIECBus->lineHigh(IECBus::Line::DATA))
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

    static quint8 previousPortA = 0xFF;
    static quint8 previousDataDirectionA = 0xFF;

    if (portA != previousPortA || dataDirectionA != previousDataDirectionA)
    {
        qDebug() << "CIA2 IEC outputs:"
                 << "PRA:" << Qt::hex << portA
                 << "DDRA:" << Qt::hex << dataDirectionA
                 << "CLK low:" << clockLow
                 << "DATA low:" << dataLow
                 << "ATN low:" << attentionLow;

        previousPortA = portA;
        previousDataDirectionA = dataDirectionA;
    }

    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::CLK, clockLow);
    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::DATA, dataLow);
    m_ptrIECBus->setLineLow(m_participantId, IECBus::Line::ATN, attentionLow);

    static quint8 previousOutputs = 0xFF;

    const quint8 outputs = portA & 0x38;

    if (outputs != previousOutputs)
    {
        previousOutputs = outputs;
    }
}
