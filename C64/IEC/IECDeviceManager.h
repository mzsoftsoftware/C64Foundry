#pragma once

#include <QHash>
#include <QtGlobal>


class IECDevice;
struct IECProtocolEvent;
struct IECProtocolResponse;


class IECDeviceManager
{
public:
    IECDeviceManager();
    ~IECDeviceManager();

    IECDeviceManager(const IECDeviceManager&) = delete;
    IECDeviceManager& operator=(const IECDeviceManager&) = delete;

    bool registerDevice(quint8 deviceNumber, IECDevice* ptrDevice);
    bool unregisterDevice(quint8 deviceNumber);

    IECDevice* device(quint8 deviceNumber) const;

    void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response);

private:
    QHash<quint8, IECDevice*> m_devices;
};
