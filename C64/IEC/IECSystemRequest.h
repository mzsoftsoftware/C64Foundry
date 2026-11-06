#pragma once

#include <QList>
#include <QSharedPointer>

#include "IECRequest.h"
#include "IECSystemResponse.h"


class IECSystem;


class IECSystemRequest : public IECRequest
{
public:
    virtual bool execute(IECSystem& system, IECSystemResponse& response) const = 0;

protected:
    IECSystemRequest() = default;
};


class IECSystemGetConfigurationsRequest : public IECSystemRequest
{
public:
    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECSystemGetConfigurationsResponse(id()));
    }

    bool execute(IECSystem& system, IECSystemResponse& response) const override;
};


class IECSystemAddDeviceRequest : public IECSystemRequest
{
public:
    explicit IECSystemAddDeviceRequest(IECDeviceType type)
        : m_deviceType(type)
    {
    }

    IECDeviceType deviceType() const { return m_deviceType; }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECSystemAddDeviceResponse(id()));
    }

    bool execute(IECSystem& system, IECSystemResponse& response) const override;

private:
    IECDeviceType m_deviceType;
};


class IECSystemRemoveDeviceRequest : public IECSystemRequest
{
public:
    explicit IECSystemRemoveDeviceRequest(IECDeviceConfiguration::DeviceId deviceId)
        : m_deviceId(deviceId)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECSystemRemoveDeviceResponse(id()));
    }

    bool execute(IECSystem& system, IECSystemResponse& response) const override;

private:
    IECDeviceConfiguration::DeviceId m_deviceId;
};


class IECSystemSetConfigurationRequest : public IECSystemRequest
{
public:
    explicit IECSystemSetConfigurationRequest(const IECDeviceConfiguration& configuration)
        : m_configuration(configuration.clone())
    {
    }

    const IECDeviceConfiguration& configuration() const { return *m_configuration; }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECSystemSetConfigurationResponse(id()));
    }

    bool execute(IECSystem& system, IECSystemResponse& response) const override;

private:
    QSharedPointer<IECDeviceConfiguration> m_configuration;
};


class IECSystemRestoreConfigurationsRequest : public IECSystemRequest
{
public:
    using ConfigurationPtr = QSharedPointer<IECDeviceConfiguration>;

    explicit IECSystemRestoreConfigurationsRequest(const QList<ConfigurationPtr>& configurations)
    {
        for (const ConfigurationPtr& ptrConfiguration : configurations)
        {
            if (ptrConfiguration.isNull())
            {
                m_configurations.append(ConfigurationPtr());
                continue;
            }

            m_configurations.append(ConfigurationPtr(ptrConfiguration->clone()));
        }
    }

    const QList<ConfigurationPtr>& configurations() const
    {
        return m_configurations;
    }

    QSharedPointer<IECResponse> createResponse() const override
    {
        return QSharedPointer<IECResponse>(new IECSystemRestoreConfigurationsResponse(id()));
    }

    bool execute(IECSystem& system, IECSystemResponse& response) const override;

private:
    QList<ConfigurationPtr> m_configurations;
};
