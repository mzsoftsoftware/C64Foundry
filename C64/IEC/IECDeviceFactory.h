#pragma once


class IECDeviceFactory
{
public:
    using DeviceId = IECDeviceConfiguration::DeviceId;

    IECDeviceFactory();
    ~IECDeviceFactory();

    IECDeviceFactory(const IECDeviceFactory&) = delete;
    IECDeviceFactory& operator=(const IECDeviceFactory&) = delete;

    IECDevice* createDevice(DeviceId id, const IECDeviceConfiguration& configuration);
    IECDevice* device(DeviceId id) const;
    bool destroyDevice(DeviceId id);
    void destroyAllDevices();

private:
    IECDevice* createPrinter(DeviceId id, const IECDeviceConfiguration& configuration);
    IECDevice* createDrive(DeviceId id, const IECDeviceConfiguration& configuration);

private:
    QHash<DeviceId, IECDevice*> m_devices;

    IECFoundryDrive* m_ptrFoundryDrive = nullptr;
};
