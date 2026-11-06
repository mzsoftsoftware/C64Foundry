#pragma once

#include <QString>
#include <QtGlobal>

#include "IECDeviceConfiguration.h"

class IECDevice;
class IECDeviceGuiResponse;


class IECDeviceGuiRequest
{
public:
    virtual ~IECDeviceGuiRequest() = default;

    IECDeviceConfiguration::DeviceId deviceId = 0;

    virtual bool execute(IECDevice& device,
                         IECDeviceGuiResponse& response) const = 0;
};

class IECDeviceGuiResponse
{
public:
    virtual ~IECDeviceGuiResponse() = default;

    IECDeviceConfiguration::DeviceId deviceId = 0;
};


class IECPrinterTearOffRequest : public IECDeviceGuiRequest
{
public:
    bool execute(IECDevice& device, IECDeviceGuiResponse& response) const override;
};
class IECPrinterTearOffResponse : public IECDeviceGuiResponse
{
public:
    bool success = false;
    QString errorMessage;
};