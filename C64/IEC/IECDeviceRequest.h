
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


class IECFoundryDriveRequest : public IECDeviceRequest
{
public:
    quint8 deviceNumber() const { return m_deviceNumber; }

protected:
    IECFoundryDriveRequest(IECDeviceConfiguration::DeviceId deviceId, quint8 deviceNumber)
        : m_deviceNumber(deviceNumber)
    {
        setDeviceId(deviceId);
    }

private:
    quint8 m_deviceNumber = 0;
};


class IECFoundryDriveSetDiskEnabledRequest : public IECFoundryDriveRequest
{
public:
    IECFoundryDriveSetDiskEnabledRequest(IECDeviceConfiguration::DeviceId deviceId, quint8 deviceNumber, bool enabled)
        : IECFoundryDriveRequest(deviceId, deviceNumber)
        , m_enabled(enabled)
    {
    }

    bool enabled() const { return m_enabled; }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECFoundryDriveResponse(id()));
    }

    bool execute(IECDevice& device, IECDeviceResponse& response) const override;

private:
    bool m_enabled = false;
};


class IECFoundryDriveInsertImageRequest : public IECFoundryDriveRequest
{
public:
    IECFoundryDriveInsertImageRequest(IECDeviceConfiguration::DeviceId deviceId, quint8 deviceNumber, const QString& fileName)
        : IECFoundryDriveRequest(deviceId, deviceNumber)
        , m_fileName(fileName)
    {
    }

    const QString& fileName() const { return m_fileName; }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECFoundryDriveResponse(id()));
    }

    bool execute(IECDevice& device, IECDeviceResponse& response) const override;

private:
    QString m_fileName;
};


class IECFoundryDriveEjectImageRequest : public IECFoundryDriveRequest
{
public:
    IECFoundryDriveEjectImageRequest(IECDeviceConfiguration::DeviceId deviceId, quint8 deviceNumber)
        : IECFoundryDriveRequest(deviceId, deviceNumber)
    {
    }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECFoundryDriveResponse(id()));
    }

    bool execute(IECDevice& device, IECDeviceResponse& response) const override;
};
