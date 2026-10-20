#pragma once

#include <QList>
#include <QtGlobal>

#include "IECDevice.h"
#include "IECDeviceConfiguration.h"
#include "IECBus.h"


class IECDeviceManager
{
public:
    using DeviceId = quint64;

    explicit IECDeviceManager(IECBus* ptrBus);
    ~IECDeviceManager();

    IECDeviceManager(const IECDeviceManager&) = delete;
    IECDeviceManager& operator=(const IECDeviceManager&) = delete;

    QList<IECDeviceType> availableDeviceTypes() const;
    QList<quint8> supportedDeviceNumbers(IECDeviceType type) const;

    QList<IECDeviceConfiguration> configurations() const;
    bool configuration(DeviceId id, IECDeviceConfiguration& configuration) const;

    DeviceId addDevice(IECDeviceType type);
    bool removeDevice(DeviceId id, IECDevice*& ptrRemovedDevice);

    bool setConfiguration(const IECDeviceConfiguration& configuration, IECDevice*& ptrRemovedDevice);

    IECDevice* device(DeviceId id) const;

    void reset();

private:
    struct DeviceEntry
    {
        IECDeviceConfiguration configuration;
        IECDevice* ptrDevice = nullptr;
    };

    DeviceEntry* findEntry(DeviceId id);
    const DeviceEntry* findEntry(DeviceId id) const;

    bool isDeviceNumberReserved(quint8 deviceNumber, DeviceId excludedId = 0) const;
    IECDevice* createDevice(const IECDeviceConfiguration& configuration) const;

    IECBus* m_ptrBus = nullptr;
    QList<DeviceEntry> m_devices;
    DeviceId m_nextDeviceId = 1;
};
