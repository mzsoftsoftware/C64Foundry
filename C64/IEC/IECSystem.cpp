#include "IECSystem.h"

#include <QQueue>

#include "IECDeviceFactory.h"
#include "IECDevice.h"
#include "IECDeviceManager.h"
#include "IECDeviceRequest.h"
#include "IECSystemRequest.h"

#include "C64/C64Machine.h"
#include "IECBus.h"
#include "IECProtocol.h"


IECSystem::IECSystem(C64Machine* ptrMachine, IECDeviceFactory* ptrDeviceFactory)
    : m_ptrMachine(ptrMachine),
    m_ptrDeviceFactory(ptrDeviceFactory)
{
    m_ptrBus = new IECBus();

    if (m_ptrMachine != nullptr)
        m_ptrMachine->setIECBus(m_ptrBus);

    m_ptrProtocol = new IECProtocol(m_ptrBus);
    m_ptrDeviceManager = new IECDeviceManager();
}
IECSystem::~IECSystem()
{
    if (m_ptrMachine != nullptr)
        m_ptrMachine->setIECBus(nullptr);

    delete m_ptrProtocol;
    m_ptrProtocol = nullptr;

    delete m_ptrDeviceManager;
    m_ptrDeviceManager = nullptr;

    delete m_ptrBus;
    m_ptrBus = nullptr;
}


void IECSystem::reset()
{
    if (m_ptrProtocol != nullptr)
        m_ptrProtocol->reset();

    if (m_ptrDeviceManager != nullptr)
        m_ptrDeviceManager->reset();
}


void IECSystem::clock()
{
    if (m_ptrProtocol == nullptr || m_ptrDeviceManager == nullptr)
        return;

    m_ptrProtocol->clock();

    QQueue<IECProtocolEvent> events = m_ptrProtocol->takeEvents();

    while (!events.isEmpty())
    {
        const IECProtocolEvent event = events.dequeue();

        IECProtocolResponse response;

        m_ptrDeviceManager->processEvent(event, response);

        if (response.type != IECProtocolResponse::Type::None)
            m_ptrProtocol->setResponse(response);
    }
}


QList<IECDeviceType> IECSystem::availableDeviceTypes() const
{
    if (m_ptrDeviceFactory == nullptr)
        return {};

    return m_ptrDeviceFactory->availableDeviceTypes();
}


QList<quint8> IECSystem::supportedDeviceNumbers(const IECDeviceType type) const
{
    if (m_ptrDeviceFactory == nullptr)
        return {};

    return m_ptrDeviceFactory->supportedDeviceNumbers(type);
}


IECSystem::DeviceEntry* IECSystem::findEntry(const DeviceId id)
{
    for (DeviceEntry& entry : m_devices)
    {
        if (entry.configuration != nullptr && entry.configuration->id() == id)
            return &entry;
    }

    return nullptr;
}


const IECSystem::DeviceEntry* IECSystem::findEntry(const DeviceId id) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration != nullptr && entry.configuration->id() == id)
            return &entry;
    }

    return nullptr;
}


IECSystem::ConfigurationPtr IECSystem::configuration(const DeviceId id) const
{
    const DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr || ptrEntry->configuration == nullptr)
        return {};

    return ConfigurationPtr(ptrEntry->configuration->clone());
}


QList<IECSystem::ConfigurationPtr> IECSystem::configurations() const
{
    QList<ConfigurationPtr> result;

    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration == nullptr)
            continue;

        result.append(ConfigurationPtr(entry.configuration->clone()));
    }

    return result;
}


void IECSystem::collectDeviceEvents()
{
    if (m_ptrDeviceManager == nullptr)
        return;

    m_deviceEvents.append(m_ptrDeviceManager->takeEvents());
}


IECSystemEventList IECSystem::takeSystemEvents()
{
    IECSystemEventList events;
    events.swap(m_systemEvents);

    return events;
}


IECDeviceEventList IECSystem::takeDeviceEvents()
{
    collectDeviceEvents();

    IECDeviceEventList events;
    events.swap(m_deviceEvents);

    return events;
}


IECResponse::Result IECSystem::processRequest(const IECRequest& request, IECResponse& response)
{
    const IECSystemRequest* ptrSystemRequest = dynamic_cast<const IECSystemRequest*>(&request);

    if (ptrSystemRequest != nullptr)
    {
        IECSystemResponse* ptrSystemResponse = dynamic_cast<IECSystemResponse*>(&response);

        if (ptrSystemResponse == nullptr)
            return IECResponse::Result::InvalidParameter;

        if (!ptrSystemRequest->execute(*this, *ptrSystemResponse))
            return IECResponse::Result::Failed;

        return IECResponse::Result::Success;
    }

    const IECDeviceRequest* ptrDeviceRequest = dynamic_cast<const IECDeviceRequest*>(&request);

    if (ptrDeviceRequest != nullptr)
    {
        IECDeviceResponse* ptrDeviceResponse = dynamic_cast<IECDeviceResponse*>(&response);

        if (ptrDeviceResponse == nullptr)
            return IECResponse::Result::InvalidParameter;

        DeviceEntry* ptrEntry = findEntry(ptrDeviceRequest->deviceId());

        if (ptrEntry == nullptr || ptrEntry->ptrDevice == nullptr)
            return IECResponse::Result::InvalidParameter;

        if (!ptrDeviceRequest->execute(*ptrEntry->ptrDevice, *ptrDeviceResponse))
            return IECResponse::Result::Failed;

        return IECResponse::Result::Success;
    }

    return IECResponse::Result::InvalidParameter;
}
