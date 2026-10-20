#include "IECDeviceManager.h"

#include "IECAsciiPrinter.h"


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

QList<quint8> IECDeviceManager::supportedDeviceNumbers(const IECDeviceType type) const
{
    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return { 4, 5 };
    }

    return {};
}

QList<IECDeviceConfiguration> IECDeviceManager::configurations() const
{
    QList<IECDeviceConfiguration> result;

    for (const DeviceEntry& entry : m_devices)
        result.append(entry.configuration);

    return result;
}

bool IECDeviceManager::configuration(const DeviceId id, IECDeviceConfiguration& configuration) const
{
    const DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr)
        return false;

    configuration = ptrEntry->configuration;
    return true;
}

IECDeviceManager::DeviceEntry* IECDeviceManager::findEntry(const DeviceId id)
{
    for (DeviceEntry& entry : m_devices)
    {
        if (entry.configuration.id == id)
            return &entry;
    }

    return nullptr;
}

const IECDeviceManager::DeviceEntry* IECDeviceManager::findEntry(const DeviceId id) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration.id == id)
            return &entry;
    }

    return nullptr;
}

bool IECDeviceManager::isDeviceNumberReserved(const quint8 deviceNumber, const DeviceId excludedId) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.configuration.id != excludedId && entry.configuration.deviceNumber == deviceNumber)
            return true;
    }

    return false;
}

IECDevice* IECDeviceManager::createDevice(const IECDeviceConfiguration& configuration) const
{
    if (m_ptrBus == nullptr)
        return nullptr;

    switch (configuration.type)
    {
    case IECDeviceType::PrinterASCII:
        return new IECAsciiPrinter(m_ptrBus, configuration.deviceNumber);
    }

    return nullptr;
}

IECDeviceManager::DeviceId IECDeviceManager::addDevice(const IECDeviceType type)
{
    const QList<quint8> numbers = supportedDeviceNumbers(type);

    for (const quint8 deviceNumber : numbers)
    {
        if (isDeviceNumberReserved(deviceNumber))
            continue;

        DeviceEntry entry;

        entry.configuration.id = m_nextDeviceId++;
        entry.configuration.type = type;
        entry.configuration.deviceNumber = deviceNumber;
        entry.configuration.enabled = false;

        m_devices.append(entry);

        return entry.configuration.id;
    }

    return 0;
}

bool IECDeviceManager::removeDevice(const DeviceId id, IECDevice*& ptrRemovedDevice)
{
    ptrRemovedDevice = nullptr;

    for (qsizetype index = 0; index < m_devices.size(); ++index)
    {
        if (m_devices[index].configuration.id != id)
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

    DeviceEntry* ptrEntry = findEntry(configuration.id);

    if (ptrEntry == nullptr)
        return false;

    const QList<quint8> supportedNumbers = supportedDeviceNumbers(configuration.type);

    if (!supportedNumbers.contains(configuration.deviceNumber))
        return false;

    if (isDeviceNumberReserved(configuration.deviceNumber, configuration.id))
        return false;

    if (ptrEntry->configuration == configuration)
        return true;

    IECDevice* ptrNewDevice = nullptr;

    if (configuration.enabled)
    {
        ptrNewDevice = createDevice(configuration);

        if (ptrNewDevice == nullptr)
            return false;
    }

    ptrRemovedDevice = ptrEntry->ptrDevice;
    ptrEntry->ptrDevice = ptrNewDevice;
    ptrEntry->configuration = configuration;

    return true;
}

IECDevice* IECDeviceManager::device(const DeviceId id) const
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
