#pragma once

#include <QString>

#include "IECResponse.h"
#include "IECDeviceConfiguration.h"


class IECDeviceResponse : public IECResponse
{
public:
    explicit IECDeviceResponse(RequestId requestId)
        : IECResponse(requestId)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }
    void setDeviceId(IECDeviceConfiguration::DeviceId deviceId) { m_deviceId = deviceId; }

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
};


class IECPrinterTearOffResponse : public IECDeviceResponse
{
public:
    explicit IECPrinterTearOffResponse(RequestId requestId)
        : IECDeviceResponse(requestId)
    {
    }

    const QString& errorMessage() const { return m_errorMessage; }
    void setErrorMessage(const QString& message) { m_errorMessage = message; }

private:
    QString m_errorMessage;
};
