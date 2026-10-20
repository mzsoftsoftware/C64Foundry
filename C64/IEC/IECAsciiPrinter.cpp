#include "IECAsciiPrinter.h"


IECAsciiPrinter::IECAsciiPrinter(IECBus* ptrBus, const quint8 deviceNumber)
    : IECPrinter(deviceNumber, ptrBus)
    , m_protocol(deviceNumber, iecBus(), participantId())
{
}

IECAsciiPrinter::~IECAsciiPrinter()
{
}

QList<quint8> IECAsciiPrinter::supportedDeviceNumbers() const
{
    return { 4, 5 };
}

void IECAsciiPrinter::reset()
{
    IECPrinter::reset();
    m_protocol.reset();
}

void IECAsciiPrinter::clock()
{
    m_protocol.clock();

    while (m_protocol.hasReceivedByte())
    {
        const IECProtocol::ReceivedByte receivedByte = m_protocol.takeReceivedByte();

        m_pendingOutput.append(static_cast<char>(receivedByte.value));

        if (receivedByte.eoi)
        {
            appendOutput(m_pendingOutput);
            m_pendingOutput.clear();
        }
    }
}

void IECAsciiPrinter::clearPendingOutput()
{
    m_pendingOutput.clear();
}
