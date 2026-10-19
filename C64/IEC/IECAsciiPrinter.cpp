#include "IECAsciiPrinter.h"


IECAsciiPrinter::IECAsciiPrinter(IECBus* ptrBus, const quint8 deviceNumber)
    : IECDevice(deviceNumber, ptrBus)
    , m_protocol(deviceNumber, iecBus(), participantId())
{
}

IECAsciiPrinter::~IECAsciiPrinter()
{
}

void IECAsciiPrinter::reset()
{
    m_protocol.reset();
    m_output.clear();
}

void IECAsciiPrinter::clock()
{
    m_protocol.clock();

    while (m_protocol.hasReceivedByte())
    {
        const IECProtocol::ReceivedByte receivedByte = m_protocol.takeReceivedByte();

        m_output.append(static_cast<char>(receivedByte.value));
    }
}

bool IECAsciiPrinter::hasOutput() const
{
    return !m_output.isEmpty();
}

QByteArray IECAsciiPrinter::takeOutput()
{
    QByteArray output;

    output.swap(m_output);

    return output;
}

void IECAsciiPrinter::clearOutput()
{
    m_output.clear();
}
