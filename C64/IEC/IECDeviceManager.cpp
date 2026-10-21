#include "IECDeviceManager.h"

#include "IECAsciiPrinter.h"
#include "IECAsciiPrinterConfiguration.h"


IECDeviceManager::IECDeviceManager(IECBus* ptrBus)
    : m_ptrBus(ptrBus)
{
}

IECDeviceManager::~IECDeviceManager()
{
    for (DeviceEntry& entry : m_devices)
        delete entry.ptrDevice;
}


QList<IECDeviceType> IECDeviceManager::availableDeviceTypes() const
{
    return { IECDeviceType::PrinterASCII };
}


QList<quint8> IECDeviceManager::supportedDeviceNumbers(IECDeviceType type) const
{
    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return { 4, 5 };
    }

    return {};
}


QList<IECDeviceManager::ConfigurationPtr> IECDeviceManager::configurations() const
{
    QList<ConfigurationPtr> result;

    for (const DeviceEntry& entry : m_devices)
        result.append(ConfigurationPtr(entry.configuration->clone()));

    return result;
}


IECDeviceManager::ConfigurationPtr IECDeviceManager::configuration(DeviceId id) const
{
    const DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr)
        return ConfigurationPtr();

    return ConfigurationPtr(ptrEntry->configuration->clone());
}


IECDeviceManager::DeviceEntry* IECDeviceManager::findEntry(DeviceId id)
{
    for (DeviceEntry& entry : m_devices)
    {
        if (entry.configuration->id() == id)
            return &entry;
    }

    return nullptr;
}


const IECDeviceManager::DeviceEntry* IECDeviceManager::findEntry(DeviceId id) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration->id() == id)
            return &entry;
    }

    return nullptr;
}


bool IECDeviceManager::isDeviceNumberReserved(quint8 deviceNumber, DeviceId excludedId) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration->id() != excludedId && entry.configuration->deviceNumber() == deviceNumber)
            return true;
    }

    return false;
}


IECDeviceManager::ConfigurationPtr IECDeviceManager::createConfiguration(IECDeviceType type) const
{
    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return ConfigurationPtr(new IECAsciiPrinterConfiguration());
    }

    return ConfigurationPtr();
}


IECDevice* IECDeviceManager::createDevice(const IECDeviceConfiguration& configuration) const
{
    if (m_ptrBus == nullptr)
        return nullptr;

    switch (configuration.type())
    {
    case IECDeviceType::PrinterASCII:
        return new IECAsciiPrinter(m_ptrBus, configuration.deviceNumber());
    }

    return nullptr;
}

QList<IECDeviceType> IECDeviceManager::addableDeviceTypes() const
{
    QList<IECDeviceType> result;

    for (IECDeviceType type : availableDeviceTypes())
    {
        if (canAddDevice(type))
            result.append(type);
    }

    return result;
}
bool IECDeviceManager::canAddDevice(IECDeviceType type) const
{
    if (!availableDeviceTypes().contains(type))
        return false;

    for (quint8 deviceNumber : supportedDeviceNumbers(type))
    {
        if (!isDeviceNumberReserved(deviceNumber))
            return true;
    }

    return false;
}
IECDeviceManager::DeviceId IECDeviceManager::addDevice(IECDeviceType type)
{
    const QList<quint8> numbers = supportedDeviceNumbers(type);

    for (quint8 deviceNumber : numbers)
    {
        if (isDeviceNumberReserved(deviceNumber))
            continue;

        ConfigurationPtr ptrConfiguration = createConfiguration(type);

        if (ptrConfiguration.isNull())
            return 0;

        const DeviceId id = m_nextDeviceId++;

        ptrConfiguration->setId(id);
        ptrConfiguration->setDeviceNumber(deviceNumber);
        ptrConfiguration->setEnabled(false);

        DeviceEntry entry;
        entry.configuration = ptrConfiguration;

        m_devices.append(entry);

        return id;
    }

    return 0;
}


bool IECDeviceManager::removeDevice(DeviceId id, IECDevice*& ptrRemovedDevice)
{
    ptrRemovedDevice = nullptr;

    for (qsizetype index = 0; index < m_devices.size(); ++index)
    {
        if (m_devices[index].configuration->id() != id)
            continue;

        ptrRemovedDevice = m_devices[index].ptrDevice;
        m_devices.removeAt(index);

        return true;
    }

    return false;
}


bool IECDeviceManager::setConfiguration(const IECDeviceConfiguration& configuration, IECDevice*& ptrRemovedDevice)
{
    ptrRemovedDevice = nullptr;

    DeviceEntry* ptrEntry = findEntry(configuration.id());

    if (ptrEntry == nullptr)
        return false;

    const QList<quint8> supportedNumbers = supportedDeviceNumbers(configuration.type());

    if (!supportedNumbers.contains(configuration.deviceNumber()))
        return false;

    if (isDeviceNumberReserved(configuration.deviceNumber(), configuration.id()))
        return false;

    // Die Geräte-ID bleibt erhalten, der Gerätetyp kann sich ändern.
    if (*ptrEntry->configuration == configuration)
        return true;

    ConfigurationPtr ptrNewConfiguration(configuration.clone());

    if (ptrNewConfiguration.isNull())
        return false;

    IECDevice* ptrNewDevice = nullptr;

    if (configuration.enabled())
    {
        ptrNewDevice = createDevice(configuration);

        if (ptrNewDevice == nullptr)
            return false;
    }

    ptrRemovedDevice = ptrEntry->ptrDevice;

    ptrEntry->ptrDevice = ptrNewDevice;
    ptrEntry->configuration = ptrNewConfiguration;

    return true;
}


IECDevice* IECDeviceManager::device(DeviceId id) const
{
    const DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr)
        return nullptr;

    return ptrEntry->ptrDevice;
}


void IECDeviceManager::reset()
{
    for (DeviceEntry& entry : m_devices)
    {
        if (entry.ptrDevice != nullptr)
            entry.ptrDevice->reset();
    }
}
