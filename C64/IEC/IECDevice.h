#pragma once

#include "IECProtocolEvent.h"
#include "IECDeviceEvent.h"


class IECDevice
{
public:
    explicit IECDevice(DeviceId id);
    virtual ~IECDevice();

    IECDevice(const IECDevice&) = delete;
    IECDevice& operator=(const IECDevice&) = delete;

    DeviceId id() const;

    virtual void reset() = 0;

    virtual void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response) = 0;

    virtual QList<quint8> supportedDeviceNumbers() const = 0;

    virtual IECDeviceEventList takeEvents() = 0;

private:
    DeviceId m_id = 0;
};
