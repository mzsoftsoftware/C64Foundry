#pragma once

#include <QList>
#include <QSharedPointer>
#include <QtGlobal>

#include "IECDevice.h"
#include "IECDeviceConfiguration.h"
#include "IECBus.h"


class IECDeviceManager
{
public:
    using DeviceId = IECDeviceConfiguration::DeviceId;
    using ConfigurationPtr = QSharedPointer<IECDeviceConfiguration>;

    explicit IECDeviceManager(IECBus* ptrBus);
    ~IECDeviceManager();

    IECDeviceManager(const IECDeviceManager&) = delete;
    IECDeviceManager& operator=(const IECDeviceManager&) = delete;

    QList<IECDeviceType> availableDeviceTypes() const;
    QList<quint8> supportedDeviceNumbers(IECDeviceType type) const;

    QList<ConfigurationPtr> configurations() const;
    ConfigurationPtr configuration(DeviceId id) const;
    bool restoreConfigurations(const QList<ConfigurationPtr>& configurations);

    QList<IECDeviceType> addableDeviceTypes() const;
    bool canAddDevice(IECDeviceType type) const;
    DeviceId addDevice(IECDeviceType type);
    bool removeDevice(DeviceId id, IECDevice*& ptrRemovedDevice);
    bool setConfiguration(const IECDeviceConfiguration& configuration, IECDevice*& ptrRemovedDevice);

    IECDevice* device(DeviceId id) const;

    void reset();

private:
    struct DeviceEntry
    {
        ConfigurationPtr configuration;
        IECDevice* ptrDevice = nullptr;
    };

    DeviceEntry* findEntry(DeviceId id);
    const DeviceEntry* findEntry(DeviceId id) const;

    bool isDeviceNumberReserved(quint8 deviceNumber, DeviceId excludedId = 0) const;

    ConfigurationPtr createConfiguration(IECDeviceType type) const;
    IECDevice* createDevice(const IECDeviceConfiguration& configuration) const;

    IECBus* m_ptrBus = nullptr;
    QList<DeviceEntry> m_devices;
    DeviceId m_nextDeviceId = 1;
};
