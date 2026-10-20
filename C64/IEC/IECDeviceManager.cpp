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

QList<IECDeviceManager::DeviceInfo> IECDeviceManager::devices() const
{
    QList<DeviceInfo> result;

    for (const DeviceEntry& entry : m_devices)
        result.append(entry.info);

    return result;
}

bool IECDeviceManager::deviceInfo(const DeviceId id, DeviceInfo& info) const
{
    const DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr)
        return false;

    info = ptrEntry->info;
    return true;
}

IECDeviceManager::DeviceEntry* IECDeviceManager::findEntry(const DeviceId id)
{
    for (DeviceEntry& entry : m_devices)
    {
        if (entry.info.id == id)
            return &entry;
    }

    return nullptr;
}

const IECDeviceManager::DeviceEntry* IECDeviceManager::findEntry(const DeviceId id) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.info.id == id)
            return &entry;
    }

    return nullptr;
}

bool IECDeviceManager::isDeviceNumberReserved(const quint8 deviceNumber, const DeviceId excludedId) const
{
    for (const DeviceEntry& entry : m_devices)
    {
        if (entry.info.id != excludedId && entry.info.deviceNumber == deviceNumber)
            return true;
    }

    return false;
}

IECDevice* IECDeviceManager::createDevice(const IECDeviceType type, const quint8 deviceNumber) const
{
    if (m_ptrBus == nullptr)
        return nullptr;

    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return new IECAsciiPrinter(m_ptrBus, deviceNumber);
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

        entry.info.id = m_nextDeviceId++;
        entry.info.type = type;
        entry.info.deviceNumber = deviceNumber;
        entry.info.enabled = false;

        m_devices.append(entry);

        return entry.info.id;
    }

    return 0;
}

bool IECDeviceManager::removeDevice(const DeviceId id, IECDevice*& ptrRemovedDevice)
{
    ptrRemovedDevice = nullptr;

    for (qsizetype index = 0; index < m_devices.size(); ++index)
    {
        if (m_devices[index].info.id != id)
            continue;

        ptrRemovedDevice = m_devices[index].ptrDevice;
        m_devices.removeAt(index);

        return true;
    }

    return false;
}

bool IECDeviceManager::setDeviceEnabled(const DeviceId id, const bool enabled, IECDevice*& ptrRemovedDevice)
{
    ptrRemovedDevice = nullptr;

    DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr)
        return false;

    if (ptrEntry->info.enabled == enabled)
        return true;

    if (enabled)
    {
        IECDevice* ptrDevice = createDevice(ptrEntry->info.type, ptrEntry->info.deviceNumber);

        if (ptrDevice == nullptr)
            return false;

        ptrEntry->ptrDevice = ptrDevice;
        ptrEntry->info.enabled = true;
    }
    else
    {
        ptrRemovedDevice = ptrEntry->ptrDevice;
        ptrEntry->ptrDevice = nullptr;
        ptrEntry->info.enabled = false;
    }

    return true;
}

bool IECDeviceManager::setDeviceNumber(const DeviceId id, const quint8 deviceNumber, IECDevice*& ptrRemovedDevice)
{
    ptrRemovedDevice = nullptr;

    DeviceEntry* ptrEntry = findEntry(id);

    if (ptrEntry == nullptr)
        return false;

    const QList<quint8> supportedNumbers = supportedDeviceNumbers(ptrEntry->info.type);

    if (!supportedNumbers.contains(deviceNumber))
        return false;

    if (isDeviceNumberReserved(deviceNumber, id))
        return false;

    if (ptrEntry->info.deviceNumber == deviceNumber)
        return true;

    if (ptrEntry->info.enabled)
    {
        IECDevice* ptrNewDevice = createDevice(ptrEntry->info.type, deviceNumber);

        if (ptrNewDevice == nullptr)
            return false;

        ptrRemovedDevice = ptrEntry->ptrDevice;
        ptrEntry->ptrDevice = ptrNewDevice;
    }

    ptrEntry->info.deviceNumber = deviceNumber;

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
