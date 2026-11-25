#include "IECPrinter.h"

#include "IECDeviceEvent.h"


IECPrinter::IECPrinter(const DeviceId id)
    : IECDevice(id)
{
}


IECPrinter::~IECPrinter() = default;



void IECPrinter::processEvent(const IECProtocolEvent& event, IECProtocolResponse& response)
{
    Q_UNUSED(response);

    switch (event.type)
    {
    case IECProtocolEvent::Type::DeviceSelected:
        m_listening = event.mode == IECProtocolEvent::Mode::Listen;
        break;

    case IECProtocolEvent::Type::ByteReceived:
        if (!m_listening)
            break;

        receiveByte(event.value);

        if (event.eoi)
            finishTransmission();

        break;

    case IECProtocolEvent::Type::EndOfTransmission:
        if (m_listening)
            finishTransmission();

        break;

    case IECProtocolEvent::Type::DeviceDeselected:
        if (m_listening)
            finishTransmission();

        m_listening = false;
        break;

    case IECProtocolEvent::Type::SecondaryAddressReceived:
    case IECProtocolEvent::Type::Open:
    case IECProtocolEvent::Type::Close:
        break;
    }
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

    QSharedPointer<IECPrinterOutputEvent> event(new IECPrinterOutputEvent());
    event->deviceId = id();
    event->format = IECPrinterOutputEvent::Format::Text;
    event->data = data;

    m_events.append(event);
}


void IECPrinter::tearOff()
{
    m_pendingOutput.clear();
}


void IECPrinter::reset()
{
    m_pendingOutput.clear();
    m_listening = false;
    m_events.clear();
}


IECDeviceEventList IECPrinter::takeEvents()
{
    IECDeviceEventList events;
    events.swap(m_events);

    return events;
}
