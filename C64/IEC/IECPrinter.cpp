#include "IECPrinter.h"


IECPrinter::IECPrinter(const quint8 deviceNumber, IECBus* ptrBus)
    : IECDevice(deviceNumber, ptrBus)
{
}

IECPrinter::~IECPrinter()
{
}

bool IECPrinter::hasOutput() const
{
    return !m_output.isEmpty();
}

quint64 IECPrinter::outputRevision() const
{
    return m_outputRevision;
}

QByteArray IECPrinter::outputSnapshot() const
{
    return m_output;
}

void IECPrinter::receiveByte(const quint8 value)
{
    m_pendingOutput.append(static_cast<char>(value));
}

void IECPrinter::finishTransmission()
{
    if (m_pendingOutput.isEmpty())
        return;

    processReceivedData(m_pendingOutput);
    m_pendingOutput.clear();
}

void IECPrinter::processReceivedData(const QByteArray& data)
{
    if (data.isEmpty())
        return;

    m_output.append(data);
    ++m_outputRevision;
}

void IECPrinter::tearOff()
{
    m_pendingOutput.clear();

    if (!m_output.isEmpty())
    {
        m_output.clear();
        ++m_outputRevision;
    }
}

void IECPrinter::reset()
{
    m_pendingOutput.clear();
    m_output.clear();
    m_outputRevision = 0;
}
