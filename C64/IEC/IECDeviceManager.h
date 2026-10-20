#pragma once

#include <QList>
#include <QtGlobal>

#include "IECDevice.h"
#include "IECBus.h"


enum class IECDeviceType
{
    PrinterASCII
};


class IECDeviceManager
{
public:
    using DeviceId = quint64;

    struct DeviceInfo
    {
        DeviceId id = 0;
        IECDeviceType type = IECDeviceType::PrinterASCII;
        quint8 deviceNumber = 0;
        bool enabled = false;
    };

    explicit IECDeviceManager(IECBus* ptrBus);
    ~IECDeviceManager();

    QList<IECDeviceType> availableDeviceTypes() const;
    QList<quint8> supportedDeviceNumbers(IECDeviceType type) const;

    QList<DeviceInfo> devices() const;
    bool deviceInfo(DeviceId id, DeviceInfo& info) const;

    DeviceId addDevice(IECDeviceType type);
    bool removeDevice(DeviceId id, IECDevice*& ptrRemovedDevice);

    bool setDeviceEnabled(DeviceId id, bool enabled, IECDevice*& ptrRemovedDevice);
    bool setDeviceNumber(DeviceId id, quint8 deviceNumber, IECDevice*& ptrRemovedDevice);

    IECDevice* device(DeviceId id) const;

    void reset();

private:
    struct DeviceEntry
    {
        DeviceInfo info;
        IECDevice* ptrDevice = nullptr;
    };

    DeviceEntry* findEntry(DeviceId id);
    const DeviceEntry* findEntry(DeviceId id) const;

    bool isDeviceNumberReserved(quint8 deviceNumber, DeviceId excludedId = 0) const;
    IECDevice* createDevice(IECDeviceType type, quint8 deviceNumber) const;

    IECBus* m_ptrBus = nullptr;
    QList<DeviceEntry> m_devices;
    DeviceId m_nextDeviceId = 1;
};
