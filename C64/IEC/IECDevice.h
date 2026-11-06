#pragma once

#include "IECProtocolEvent.h"
#include "IECDeviceEvent.h"


class IECDevice
{
public:
    virtual ~IECDevice() = default;

    IECDevice(const IECDevice&) = delete;
    IECDevice& operator=(const IECDevice&) = delete;

    virtual void reset() = 0;

    virtual void handleProtocolEvent(const IECProtocolEvent& event) = 0;

    virtual IECDeviceEventList takeEvents();

protected:
    IECDevice() = default;
};
