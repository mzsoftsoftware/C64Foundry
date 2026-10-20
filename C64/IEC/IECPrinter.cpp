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

void IECPrinter::appendOutput(const QByteArray& data)
{
    if (data.isEmpty())
        return;

    m_output.append(data);
    ++m_outputRevision;
}

void IECPrinter::clearPendingOutput()
{
}

void IECPrinter::tearOff()
{
    clearPendingOutput();

    if (!m_output.isEmpty())
    {
        m_output.clear();
        ++m_outputRevision;
    }
}

void IECPrinter::reset()
{
    tearOff();
}
