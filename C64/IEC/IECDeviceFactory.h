#pragma once

#include <QHash>

#include "IECDeviceConfiguration.h"


class IECDevice;
class IECFoundryDrive;


class IECDeviceFactory
{
public:
    using DeviceId = IECDeviceConfiguration::DeviceId;

    IECDeviceFactory();
    ~IECDeviceFactory();

    IECDeviceFactory(const IECDeviceFactory&) = delete;
    IECDeviceFactory& operator=(const IECDeviceFactory&) = delete;

    IECDevice* createDevice(const IECDeviceConfiguration& configuration);
    IECDevice* device(DeviceId id) const;
    bool destroyDevice(DeviceId id);
    void destroyAllDevices();

private:
    IECDevice* createPrinter(const IECDeviceConfiguration& configuration);
    IECDevice* createDrive(const IECDeviceConfiguration& configuration);

private:
    QHash<DeviceId, IECDevice*> m_devices;

    IECFoundryDrive* m_ptrFoundryDrive = nullptr;
};
