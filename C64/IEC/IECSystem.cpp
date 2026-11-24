#include "IECSystem.h"

#include <QQueue>
#include <QStringList>

#include "IECConfigurationValidator.h"
#include "IECDevice.h"
#include "IECDeviceFactory.h"
#include "IECDeviceManager.h"
#include "IECDeviceRequest.h"
#include "IECSystemRequest.h"

#include "IECAsciiPrinter.h"
#include "IECAsciiPrinterConfiguration.h"
#include "IECFoundryDrive.h"
#include "IECFoundryDriveConfiguration.h"
#include "IECFoundryDiskConfiguration.h"

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
    for (DeviceEntry& entry : m_devices)
        destroyDevice(entry);

    m_devices.clear();

    if (m_ptrMachine != nullptr)
        m_ptrMachine->setIECBus(nullptr);

    delete m_ptrProtocol;
    m_ptrProtocol = nullptr;

    delete m_ptrDeviceManager;
    m_ptrDeviceManager = nullptr;

    delete m_ptrBus;
    m_ptrBus = nullptr;
}


//
// Lifecycle and clocking
//

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


//
// Device types and supported addresses
//

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


QList<IECDeviceType> IECSystem::addableDeviceTypes() const
{
    QList<IECDeviceType> result;

    for (IECDeviceType type : availableDeviceTypes())
    {
        if (canAddDevice(type))
            result.append(type);
    }

    return result;
}


bool IECSystem::canAddDevice(const IECDeviceType type) const
{
    if (m_ptrDeviceFactory == nullptr || !availableDeviceTypes().contains(type))
        return false;

    const QList<quint8> supportedNumbers =
        m_ptrDeviceFactory->supportedDeviceNumbers(type);

    for (quint8 deviceNumber : supportedNumbers)
    {
        if (!isDeviceNumberReserved(deviceNumber))
            return true;
    }

    return false;
}


//
// Device lookup
//

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


//
// Configurations
//

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


//
// Address reservation
//

bool IECSystem::isDeviceNumberReserved(
    const quint8 deviceNumber,
    const DeviceId excludedId) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration == nullptr ||
            entry.configuration->id() == excludedId)
        {
            continue;
        }

        if (entry.configuration->type() == IECDeviceType::PrinterASCII)
        {
            const IECAsciiPrinterConfiguration* ptrConfiguration =
                dynamic_cast<const IECAsciiPrinterConfiguration*>(
                    entry.configuration.data());

            if (ptrConfiguration != nullptr &&
                ptrConfiguration->deviceNumber() == deviceNumber)
            {
                return true;
            }
        }
        else if (entry.configuration->type() == IECDeviceType::FoundryDrive)
        {
            const IECFoundryDriveConfiguration* ptrConfiguration =
                dynamic_cast<const IECFoundryDriveConfiguration*>(
                    entry.configuration.data());

            if (ptrConfiguration != nullptr &&
                deviceNumber >= ptrConfiguration->startDeviceNumber() &&
                deviceNumber <= ptrConfiguration->endDeviceNumber())
            {
                return true;
            }
        }
    }

    return false;
}


//
// Device registration
//

bool IECSystem::registerDevice(DeviceEntry& entry)
{
    if (m_ptrDeviceManager == nullptr ||
        entry.configuration == nullptr ||
        entry.ptrDevice == nullptr)
    {
        return false;
    }

    if (!entry.configuration->enabled())
        return true;

    QList<quint8> deviceNumbers;

    if (entry.configuration->type() == IECDeviceType::PrinterASCII)
    {
        const IECAsciiPrinterConfiguration* ptrConfiguration =
            dynamic_cast<const IECAsciiPrinterConfiguration*>(
                entry.configuration.data());

        if (ptrConfiguration == nullptr)
            return false;

        deviceNumbers.append(ptrConfiguration->deviceNumber());
    }
    else if (entry.configuration->type() == IECDeviceType::FoundryDrive)
    {
        const IECFoundryDriveConfiguration* ptrConfiguration =
            dynamic_cast<const IECFoundryDriveConfiguration*>(
                entry.configuration.data());

        if (ptrConfiguration == nullptr)
            return false;

        for (int deviceNumber = ptrConfiguration->startDeviceNumber();
             deviceNumber <= ptrConfiguration->endDeviceNumber();
             ++deviceNumber)
        {
            deviceNumbers.append(static_cast<quint8>(deviceNumber));
        }
    }
    else
    {
        return false;
    }

    QList<quint8> registeredNumbers;

    for (quint8 deviceNumber : deviceNumbers)
    {
        if (!m_ptrDeviceManager->registerDevice(deviceNumber, entry.ptrDevice))
        {
            for (quint8 registeredNumber : registeredNumbers)
            {
                m_ptrDeviceManager->unregisterDevice(
                    registeredNumber, entry.ptrDevice);
            }

            return false;
        }

        registeredNumbers.append(deviceNumber);
    }

    return true;
}


void IECSystem::unregisterDevice(DeviceEntry& entry)
{
    if (m_ptrDeviceManager == nullptr ||
        entry.configuration == nullptr ||
        entry.ptrDevice == nullptr)
    {
        return;
    }

    if (entry.configuration->type() == IECDeviceType::PrinterASCII)
    {
        const IECAsciiPrinterConfiguration* ptrConfiguration =
            dynamic_cast<const IECAsciiPrinterConfiguration*>(
                entry.configuration.data());

        if (ptrConfiguration != nullptr)
        {
            m_ptrDeviceManager->unregisterDevice(
                ptrConfiguration->deviceNumber(), entry.ptrDevice);
        }
    }
    else if (entry.configuration->type() == IECDeviceType::FoundryDrive)
    {
        const IECFoundryDriveConfiguration* ptrConfiguration =
            dynamic_cast<const IECFoundryDriveConfiguration*>(
                entry.configuration.data());

        if (ptrConfiguration != nullptr)
        {
            for (int deviceNumber = ptrConfiguration->startDeviceNumber();
                 deviceNumber <= ptrConfiguration->endDeviceNumber();
                 ++deviceNumber)
            {
                m_ptrDeviceManager->unregisterDevice(
                    static_cast<quint8>(deviceNumber), entry.ptrDevice);
            }
        }
    }
}


void IECSystem::destroyDevice(DeviceEntry& entry)
{
    unregisterDevice(entry);

    if (m_ptrDeviceFactory != nullptr && entry.configuration != nullptr)
        m_ptrDeviceFactory->destroyDevice(entry.configuration->id());

    entry.ptrDevice = nullptr;
}


//
// Device management
//

IECSystem::DeviceId IECSystem::addDevice(const IECDeviceType type)
{
    if (!canAddDevice(type) || m_ptrDeviceFactory == nullptr)
        return 0;

    ConfigurationPtr ptrConfiguration;

    if (type == IECDeviceType::PrinterASCII)
    {
        IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
            new IECAsciiPrinterConfiguration();

        for (quint8 deviceNumber : supportedDeviceNumbers(type))
        {
            if (!isDeviceNumberReserved(deviceNumber))
            {
                ptrPrinterConfiguration->setDeviceNumber(deviceNumber);
                break;
            }
        }

        ptrConfiguration.reset(ptrPrinterConfiguration);
    }
    else if (type == IECDeviceType::FoundryDrive)
    {
        IECFoundryDriveConfiguration* ptrDriveConfiguration =
            new IECFoundryDriveConfiguration();

        quint8 startDeviceNumber = 0;

        for (quint8 deviceNumber : supportedDeviceNumbers(type))
        {
            if (!isDeviceNumberReserved(deviceNumber))
            {
                startDeviceNumber = deviceNumber;
                break;
            }
        }

        if (startDeviceNumber == 0)
        {
            delete ptrDriveConfiguration;
            return 0;
        }

        ptrDriveConfiguration->setStartDeviceNumber(startDeviceNumber);

        IECFoundryDiskConfiguration diskConfiguration;
        diskConfiguration.setSlotIndex(0);
        diskConfiguration.setDeviceNumber(startDeviceNumber);
        diskConfiguration.setEnabled(true);

        if (!ptrDriveConfiguration->addDiskConfiguration(diskConfiguration))
        {
            delete ptrDriveConfiguration;
            return 0;
        }

        ptrConfiguration.reset(ptrDriveConfiguration);
    }
    else
    {
        return 0;
    }

    const DeviceId deviceId = m_nextDeviceId++;

    ptrConfiguration->setId(deviceId);
    ptrConfiguration->setEnabled(true);

    IECDevice* ptrDevice =
        m_ptrDeviceFactory->createDevice(*ptrConfiguration);

    if (ptrDevice == nullptr)
        return 0;

    DeviceEntry entry;
    entry.configuration = ptrConfiguration;
    entry.ptrDevice = ptrDevice;

    if (!registerDevice(entry))
    {
        m_ptrDeviceFactory->destroyDevice(deviceId);
        return 0;
    }

    m_devices.append(entry);

    addSystemEvent(IECSystemEvent::Type::DeviceAdded, deviceId);

    return deviceId;
}


bool IECSystem::removeDevice(const DeviceId id)
{
    for (int index = 0; index < m_devices.size(); ++index)
    {
        if (m_devices.at(index).configuration == nullptr ||
            m_devices.at(index).configuration->id() != id)
        {
            continue;
        }

        destroyDevice(m_devices[index]);
        m_devices.removeAt(index);

        addSystemEvent(IECSystemEvent::Type::DeviceRemoved, id);

        return true;
    }

    return false;
}


bool IECSystem::setConfiguration(
    const IECDeviceConfiguration& configuration,
    QString* operationMessage)
{
    if (operationMessage != nullptr)
        operationMessage->clear();

    DeviceEntry* ptrEntry = findEntry(configuration.id());

    if (ptrEntry == nullptr || ptrEntry->ptrDevice == nullptr)
        return false;

    if (configuration.type() != ptrEntry->configuration->type())
        return false;

    QList<ConfigurationPtr> configurationsToValidate;

    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration == nullptr)
            continue;

        if (entry.configuration->id() == configuration.id())
        {
            configurationsToValidate.append(
                ConfigurationPtr(configuration.clone()));
        }
        else
        {
            configurationsToValidate.append(
                ConfigurationPtr(entry.configuration->clone()));
        }
    }

    QStringList errors;

    if (!IECConfigurationValidator::validate(
            configurationsToValidate, &errors))
    {
        if (operationMessage != nullptr)
            *operationMessage = errors.join('\n');

        return false;
    }

    ConfigurationPtr previousConfiguration =
        ptrEntry->configuration;

    unregisterDevice(*ptrEntry);

    bool success = false;

    if (configuration.type() == IECDeviceType::PrinterASCII)
    {
        IECAsciiPrinter* ptrPrinter =
            dynamic_cast<IECAsciiPrinter*>(ptrEntry->ptrDevice);

        const IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
            dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

        success = ptrPrinter != nullptr &&
                  ptrPrinterConfiguration != nullptr &&
                  ptrPrinter->setConfiguration(*ptrPrinterConfiguration);
    }
    else if (configuration.type() == IECDeviceType::FoundryDrive)
    {
        IECFoundryDrive* ptrDrive =
            dynamic_cast<IECFoundryDrive*>(ptrEntry->ptrDevice);

        const IECFoundryDriveConfiguration* ptrDriveConfiguration =
            dynamic_cast<const IECFoundryDriveConfiguration*>(&configuration);

        success = ptrDrive != nullptr &&
                  ptrDriveConfiguration != nullptr &&
                  ptrDrive->setConfiguration(*ptrDriveConfiguration);
    }

    if (!success)
    {
        if (operationMessage != nullptr)
            *operationMessage =
                QStringLiteral("The device rejected the configuration.");

        if (configuration.type() == IECDeviceType::FoundryDrive)
        {
            IECFoundryDrive* ptrDrive =
                dynamic_cast<IECFoundryDrive*>(ptrEntry->ptrDevice);

            const IECFoundryDriveConfiguration* ptrPreviousConfiguration =
                dynamic_cast<const IECFoundryDriveConfiguration*>(
                    previousConfiguration.data());

            if (ptrDrive != nullptr && ptrPreviousConfiguration != nullptr)
                ptrDrive->setConfiguration(*ptrPreviousConfiguration);
        }

        registerDevice(*ptrEntry);
        return false;
    }

    ptrEntry->configuration.reset(configuration.clone());

    if (!registerDevice(*ptrEntry))
    {
        if (operationMessage != nullptr)
            *operationMessage =
                QStringLiteral("The device addresses could not be registered.");

        if (configuration.type() == IECDeviceType::FoundryDrive)
        {
            IECFoundryDrive* ptrDrive =
                dynamic_cast<IECFoundryDrive*>(ptrEntry->ptrDevice);

            const IECFoundryDriveConfiguration* ptrPreviousConfiguration =
                dynamic_cast<const IECFoundryDriveConfiguration*>(
                    previousConfiguration.data());

            if (ptrDrive != nullptr && ptrPreviousConfiguration != nullptr)
                ptrDrive->setConfiguration(*ptrPreviousConfiguration);
        }

        ptrEntry->configuration = previousConfiguration;
        registerDevice(*ptrEntry);

        return false;
    }

    addSystemEvent(
        IECSystemEvent::Type::ConfigurationChanged,
        configuration.id());

    return true;
}


//
// Restore configurations
//

bool IECSystem::restoreConfigurations(
    const QList<ConfigurationPtr>& configurations)
{
    QStringList errors;

    if (!IECConfigurationValidator::validate(configurations, &errors))
        return false;

    if (m_ptrDeviceFactory == nullptr || m_ptrDeviceManager == nullptr)
        return false;

    for (DeviceEntry& entry : m_devices)
        destroyDevice(entry);

    m_devices.clear();

    DeviceId nextDeviceId = 1;

    for (const ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
            return false;

        IECDevice* ptrDevice =
            m_ptrDeviceFactory->createDevice(*ptrConfiguration);

        if (ptrDevice == nullptr)
        {
            for (DeviceEntry& entry : m_devices)
                destroyDevice(entry);

            m_devices.clear();
            return false;
        }

        DeviceEntry entry;
        entry.configuration.reset(ptrConfiguration->clone());
        entry.ptrDevice = ptrDevice;

        if (!registerDevice(entry))
        {
            m_ptrDeviceFactory->destroyDevice(ptrConfiguration->id());

            for (DeviceEntry& existingEntry : m_devices)
                destroyDevice(existingEntry);

            m_devices.clear();
            return false;
        }

        m_devices.append(entry);

        nextDeviceId = qMax(
            nextDeviceId, ptrConfiguration->id() + 1);
    }

    m_nextDeviceId = nextDeviceId;

    addSystemEvent(IECSystemEvent::Type::ConfigurationsRestored);

    return true;
}


//
// Events
//

void IECSystem::addSystemEvent(
    const IECSystemEvent::Type type,
    const DeviceId deviceId)
{
    IECSystemEventPtr event(new IECSystemEvent());
    event->type = type;
    event->deviceId = deviceId;

    m_systemEvents.append(event);
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


//
// Requests
//

IECResponse::Result IECSystem::processRequest(
    const IECRequest& request,
    IECResponse& response)
{
    const IECSystemRequest* ptrSystemRequest =
        dynamic_cast<const IECSystemRequest*>(&request);

    if (ptrSystemRequest != nullptr)
    {
        IECSystemResponse* ptrSystemResponse =
            dynamic_cast<IECSystemResponse*>(&response);

        if (ptrSystemResponse == nullptr)
            return IECResponse::Result::InvalidParameter;

        if (!ptrSystemRequest->execute(*this, *ptrSystemResponse))
            return IECResponse::Result::Failed;

        return IECResponse::Result::Success;
    }

    const IECDeviceRequest* ptrDeviceRequest =
        dynamic_cast<const IECDeviceRequest*>(&request);

    if (ptrDeviceRequest != nullptr)
    {
        IECDeviceResponse* ptrDeviceResponse =
            dynamic_cast<IECDeviceResponse*>(&response);

        if (ptrDeviceResponse == nullptr)
            return IECResponse::Result::InvalidParameter;

        DeviceEntry* ptrEntry =
            findEntry(ptrDeviceRequest->deviceId());

        if (ptrEntry == nullptr || ptrEntry->ptrDevice == nullptr)
            return IECResponse::Result::InvalidParameter;

        if (!ptrDeviceRequest->execute(
                *ptrEntry->ptrDevice, *ptrDeviceResponse))
        {
            return IECResponse::Result::Failed;
        }

        return IECResponse::Result::Success;
    }

    return IECResponse::Result::InvalidParameter;
}
