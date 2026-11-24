#pragma once

#include <QtGlobal>

#include "IECDeviceEvent.h"


class IECDevice;
struct IECProtocolEvent;
struct IECProtocolResponse;


class IECDeviceManager
{
public:
    static constexpr quint8 MAX_DEVICE_NUMBER = 30;
    static constexpr quint8 DEVICE_NUMBER_COUNT = MAX_DEVICE_NUMBER + 1;

    IECDeviceManager() = default;
    ~IECDeviceManager() = default;

    IECDeviceManager(const IECDeviceManager&) = delete;
    IECDeviceManager& operator=(const IECDeviceManager&) = delete;

    bool registerDevice(quint8 deviceNumber, IECDevice* ptrDevice);
    bool unregisterDevice(quint8 deviceNumber, IECDevice* ptrDevice);

    IECDevice* device(quint8 deviceNumber) const;

    void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response);

    void reset();
    IECDeviceEventList takeEvents();

private:
    IECDevice* m_devices[DEVICE_NUMBER_COUNT] = {};
};
