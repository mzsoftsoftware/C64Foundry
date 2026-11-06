#pragma once

#include "IECRequest.h"
#include "IECDeviceResponse.h"

class IECDevice;


class IECDeviceRequest : public IECRequest
{
public:
    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }
    void setDeviceId(IECDeviceConfiguration::DeviceId deviceId) { m_deviceId = deviceId; }

    virtual bool execute(IECDevice& device, IECDeviceResponse& response) const = 0;

protected:
    IECDeviceRequest() = default;

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
};


class IECPrinterTearOffRequest : public IECDeviceRequest
{
public:
    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECPrinterTearOffResponse(id()));
    }

    bool execute(IECDevice& device, IECDeviceResponse& response) const override;
};
