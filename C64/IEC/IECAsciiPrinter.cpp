#include "IECAsciiPrinter.h"

#include "IECAsciiPrinterConfiguration.h"


IECAsciiPrinter::IECAsciiPrinter(IECBus* ptrBus, const quint8 deviceNumber)
    : IECPrinter(ptrBus, deviceNumber)
    , m_protocol(iecBus(), deviceNumber, participantId())
{
}

IECAsciiPrinter::~IECAsciiPrinter()
{
}

QList<quint8> IECAsciiPrinter::supportedDeviceNumbers()
{
    return { 4, 5 };
}

bool IECAsciiPrinter::setConfiguration(const IECAsciiPrinterConfiguration& configuration)
{
    if (configuration.type() != IECDeviceType::PrinterASCII)
        return false;

    if (configuration.deviceNumber() != deviceNumber())
        return false;

    if (configuration.lineWidth() < 1)
        return false;

    m_lineWidth = configuration.lineWidth();
    m_autoWrap = configuration.autoWrap();

    return true;
}

void IECAsciiPrinter::tearOff()
{
    IECPrinter::tearOff();

    m_column = 0;
    m_lastWasCR = false;
}

void IECAsciiPrinter::reset()
{
    IECPrinter::reset();
    m_protocol.reset();

    m_column = 0;
    m_lastWasCR = false;
}

void IECAsciiPrinter::clock()
{
    m_protocol.clock();

    while (m_protocol.hasReceivedByte())
    {
        const IECProtocol::ReceivedByte receivedByte = m_protocol.takeReceivedByte();

        receiveByte(receivedByte.value);

        if (receivedByte.eoi)
            finishTransmission();
    }

    if (m_protocol.takeListenEnded())
        finishTransmission();
}

void IECAsciiPrinter::processReceivedData(const QByteArray& data)
{
    if (data.isEmpty())
        return;

    QByteArray output;

    for (char character : data)
    {
        const quint8 value = static_cast<quint8>(character);

        if (value == 0x0D)
        {
            output.append('\n');
            m_column = 0;
            m_lastWasCR = true;
            continue;
        }

        if (value == 0x0A)
        {
            if (!m_lastWasCR)
                output.append('\n');

            m_column = 0;
            m_lastWasCR = false;
            continue;
        }

        m_lastWasCR = false;

        if (m_autoWrap && m_column >= m_lineWidth)
        {
            output.append('\n');
            m_column = 0;
        }

        output.append(character);
        ++m_column;
    }

    IECPrinter::processReceivedData(output);
}
