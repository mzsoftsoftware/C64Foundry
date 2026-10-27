#include "IECDeviceManager.h"

#include "IECAsciiPrinter.h"
#include "IECAsciiPrinterConfiguration.h"

#include "IECFoundryDisk.h"
#include "IECFoundryDiskConfiguration.h"


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
    case IECDeviceType::FoundryDisk:
        return { 8, 9, 10, 11 };
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

bool IECDeviceManager::restoreConfigurations(const QList<ConfigurationPtr>& configurations)
{
    // Wiederherstellung ist nur für einen noch leeren Manager vorgesehen.
    if (!m_devices.isEmpty())
        return false;

    QList<DeviceEntry> restoredDevices;
    QList<DeviceId> usedIds;
    QList<quint8> usedDeviceNumbers;

    DeviceId highestId = 0;
    bool bValid = true;

    for (const ConfigurationPtr& ptrConfiguration : configurations)
    {
        if (ptrConfiguration.isNull())
        {
            bValid = false;
            break;
        }

        const DeviceId id = ptrConfiguration->id();
        const IECDeviceType type = ptrConfiguration->type();
        const quint8 deviceNumber = ptrConfiguration->deviceNumber();

        if (id == 0 || id == static_cast<DeviceId>(-1))
        {
            bValid = false;
            break;
        }

        if (!availableDeviceTypes().contains(type))
        {
            bValid = false;
            break;
        }

        if (!supportedDeviceNumbers(type).contains(deviceNumber))
        {
            bValid = false;
            break;
        }

        if (usedIds.contains(id) || usedDeviceNumbers.contains(deviceNumber))
        {
            bValid = false;
            break;
        }

        ConfigurationPtr ptrCopy(ptrConfiguration->clone());

        if (ptrCopy.isNull())
        {
            bValid = false;
            break;
        }

        // Gerätespezifische Einstellungen auch bei ausgeschalteten Geräten prüfen.
        switch (type)
        {
        case IECDeviceType::PrinterASCII:
        {
            const IECAsciiPrinterConfiguration* ptrPrinterConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(ptrCopy.data());

            if (ptrPrinterConfiguration == nullptr || ptrPrinterConfiguration->lineWidth() < 1)
                bValid = false;

            break;
        }
        case IECDeviceType::FoundryDisk:
        {
            const IECFoundryDiskConfiguration* ptrDiskConfiguration = dynamic_cast<const IECFoundryDiskConfiguration*>(ptrCopy.data());

            if (ptrDiskConfiguration == nullptr)
                bValid = false;

            break;
        }
        }

        if (!bValid)
            break;

        DeviceEntry entry;
        entry.configuration = ptrCopy;

        if (ptrCopy->enabled())
        {
            entry.ptrDevice = createDevice(*ptrCopy);

            if (entry.ptrDevice == nullptr)
            {
                bValid = false;
                break;
            }
        }

        restoredDevices.append(entry);
        usedIds.append(id);
        usedDeviceNumbers.append(deviceNumber);

        if (id > highestId)
            highestId = id;
    }

    if (!bValid)
    {
        // Temporär erzeugte Geräte freigeben.
        for (DeviceEntry& entry : restoredDevices)
            delete entry.ptrDevice;

        return false;
    }

    // Erst jetzt den geprüften Zustand übernehmen.
    m_devices.swap(restoredDevices);
    m_nextDeviceId = highestId + 1;

    return true;
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
    case IECDeviceType::FoundryDisk:
        return ConfigurationPtr(new IECFoundryDiskConfiguration());
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
    {
        const IECAsciiPrinterConfiguration* ptrConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

        if (ptrConfiguration == nullptr)
            return nullptr;

        IECAsciiPrinter* ptrPrinter = new IECAsciiPrinter(m_ptrBus, configuration.deviceNumber());

        if (!ptrPrinter->setConfiguration(*ptrConfiguration))
        {
            delete ptrPrinter;
            return nullptr;
        }

        return ptrPrinter;
    }
    case IECDeviceType::FoundryDisk:
    {
        const IECFoundryDiskConfiguration* ptrConfiguration = dynamic_cast<const IECFoundryDiskConfiguration*>(&configuration);

        if (ptrConfiguration == nullptr)
            return nullptr;

        IECFoundryDisk* ptrDisk = new IECFoundryDisk(m_ptrBus, configuration.deviceNumber());

        if (!ptrDisk->setConfiguration(*ptrConfiguration))
        {
            delete ptrDisk;
            return nullptr;
        }

        return ptrDisk;
    }
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

    if (ptrEntry->configuration->type() != configuration.type())
        return false;

    const QList<quint8> supportedNumbers = supportedDeviceNumbers(configuration.type());

    if (!supportedNumbers.contains(configuration.deviceNumber()))
        return false;

    if (isDeviceNumberReserved(configuration.deviceNumber(), configuration.id()))
        return false;

    if (*ptrEntry->configuration == configuration)
        return true;

    ConfigurationPtr ptrNewConfiguration(configuration.clone());

    if (ptrNewConfiguration.isNull())
        return false;

    const bool bRecreateDevice =
        ptrEntry->configuration->deviceNumber() != configuration.deviceNumber() ||
        ptrEntry->configuration->enabled() != configuration.enabled();

    if (!bRecreateDevice)
    {
        if (ptrEntry->ptrDevice != nullptr)
        {
            switch (configuration.type())
            {
            case IECDeviceType::PrinterASCII:
            {
                IECAsciiPrinter* ptrPrinter = dynamic_cast<IECAsciiPrinter*>(ptrEntry->ptrDevice);
                const IECAsciiPrinterConfiguration* ptrPrinterConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

                if (ptrPrinter == nullptr || ptrPrinterConfiguration == nullptr)
                    return false;

                if (!ptrPrinter->setConfiguration(*ptrPrinterConfiguration))
                    return false;

                break;
            }
            case IECDeviceType::FoundryDisk:
            {
                IECFoundryDisk* ptrDisk = dynamic_cast<IECFoundryDisk*>(ptrEntry->ptrDevice);
                const IECFoundryDiskConfiguration* ptrDiskConfiguration = dynamic_cast<const IECFoundryDiskConfiguration*>(&configuration);

                if (ptrDisk == nullptr || ptrDiskConfiguration == nullptr)
                    return false;

                if (!ptrDisk->setConfiguration(*ptrDiskConfiguration))
                    return false;

                break;
            }
            }
        }
        else
        {
            // Auch ausgeschaltete Geräte müssen gültige Einstellungen besitzen.
            switch (configuration.type())
            {
            case IECDeviceType::PrinterASCII:
            {
                const IECAsciiPrinterConfiguration* ptrPrinterConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

                if (ptrPrinterConfiguration == nullptr || ptrPrinterConfiguration->lineWidth() < 1)
                    return false;

                break;
            }
            case IECDeviceType::FoundryDisk:
            {
                const IECFoundryDiskConfiguration* ptrDiskConfiguration = dynamic_cast<const IECFoundryDiskConfiguration*>(&configuration);

                if (ptrDiskConfiguration == nullptr)
                    return false;

                break;
            }
            }
        }

        ptrEntry->configuration = ptrNewConfiguration;
        return true;
    }

    IECDevice* ptrNewDevice = nullptr;

    if (configuration.enabled())
    {
        ptrNewDevice = createDevice(configuration);

        if (ptrNewDevice == nullptr)
            return false;
    }
    else if (configuration.type() == IECDeviceType::PrinterASCII)
    {
        const IECAsciiPrinterConfiguration* ptrPrinterConfiguration = dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

        if (ptrPrinterConfiguration == nullptr || ptrPrinterConfiguration->lineWidth() < 1)
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
