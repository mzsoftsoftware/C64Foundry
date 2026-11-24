#include "IECDeviceFactory.h"

#include "IECDevice.h"
#include "IECAsciiPrinter.h"
#include "IECAsciiPrinterConfiguration.h"
#include "IECFoundryDrive.h"
#include "IECFoundryDriveConfiguration.h"


IECDeviceFactory::IECDeviceFactory()
{
}


IECDeviceFactory::~IECDeviceFactory()
{
    destroyAllDevices();
}


QList<IECDeviceType> IECDeviceFactory::availableDeviceTypes() const
{
    return {
        IECDeviceType::PrinterASCII,
        IECDeviceType::FoundryDrive
    };
}


QList<quint8> IECDeviceFactory::supportedDeviceNumbers(const IECDeviceType type) const
{
    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return { 4, 5 };

    case IECDeviceType::FoundryDrive:
    {
        QList<quint8> deviceNumbers;

        for (quint8 deviceNumber = 4; deviceNumber <= 30; ++deviceNumber)
            deviceNumbers.append(deviceNumber);

        return deviceNumbers;
    }
    }

    return {};
}


IECDevice* IECDeviceFactory::createDevice(const IECDeviceConfiguration& configuration)
{
    if (configuration.id() == 0 || m_devices.contains(configuration.id()))
        return nullptr;

    IECDevice* ptrDevice = nullptr;

    switch (configuration.type())
    {
    case IECDeviceType::PrinterASCII:
        ptrDevice = createPrinter(configuration);
        break;

    case IECDeviceType::FoundryDrive:
        ptrDevice = createDrive(configuration);
        break;
    }

    if (ptrDevice == nullptr)
        return nullptr;

    m_devices.insert(configuration.id(), ptrDevice);

    return ptrDevice;
}


IECDevice* IECDeviceFactory::device(const DeviceId id) const
{
    return m_devices.value(id, nullptr);
}


bool IECDeviceFactory::destroyDevice(const DeviceId id)
{
    IECDevice* ptrDevice = m_devices.take(id);

    if (ptrDevice == nullptr)
        return false;

    delete ptrDevice;

    return true;
}


void IECDeviceFactory::destroyAllDevices()
{
    for (IECDevice* ptrDevice : m_devices)
        delete ptrDevice;

    m_devices.clear();
}


IECDevice* IECDeviceFactory::createPrinter(const IECDeviceConfiguration& configuration)
{
    const IECAsciiPrinterConfiguration* ptrPrinterConfiguration =
        dynamic_cast<const IECAsciiPrinterConfiguration*>(&configuration);

    if (ptrPrinterConfiguration == nullptr)
        return nullptr;

    IECAsciiPrinter* ptrPrinter = new IECAsciiPrinter(configuration.id());

    if (!ptrPrinter->setConfiguration(*ptrPrinterConfiguration))
    {
        delete ptrPrinter;
        return nullptr;
    }

    return ptrPrinter;
}


IECDevice* IECDeviceFactory::createDrive(const IECDeviceConfiguration& configuration)
{
    const IECFoundryDriveConfiguration* ptrDriveConfiguration =
        dynamic_cast<const IECFoundryDriveConfiguration*>(&configuration);

    if (ptrDriveConfiguration == nullptr)
        return nullptr;

    IECFoundryDrive* ptrDrive = new IECFoundryDrive(configuration.id());

    if (!ptrDrive->setConfiguration(*ptrDriveConfiguration))
    {
        delete ptrDrive;
        return nullptr;
    }

    return ptrDrive;
}
