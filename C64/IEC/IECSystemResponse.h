#pragma once

#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>

#include "IECResponse.h"
#include "IECDeviceConfiguration.h"


class IECSystemResponse : public IECResponse
{
public:
    explicit IECSystemResponse(RequestId requestId)
        : IECResponse(requestId)
    {
    }
};


class IECSystemGetConfigurationsResponse : public IECSystemResponse
{
public:
    explicit IECSystemGetConfigurationsResponse(RequestId requestId)
        : IECSystemResponse(requestId)
    {
    }

    const QList<QSharedPointer<IECDeviceConfiguration>>& configurations() const
    {
        return m_configurations;
    }

    const QList<IECDeviceType>& availableDeviceTypes() const
    {
        return m_availableDeviceTypes;
    }

    const QList<IECDeviceType>& addableDeviceTypes() const
    {
        return m_addableDeviceTypes;
    }

    const QHash<IECDeviceType, QList<quint8>>& supportedDeviceNumbers() const
    {
        return m_supportedDeviceNumbers;
    }

    void setConfigurations(const QList<QSharedPointer<IECDeviceConfiguration>>& configurations)
    {
        m_configurations.clear();

        for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration : configurations)
        {
            if (!ptrConfiguration.isNull())
                m_configurations.append(QSharedPointer<IECDeviceConfiguration>(ptrConfiguration->clone()));
        }
    }

    void setAvailableDeviceTypes(const QList<IECDeviceType>& types)
    {
        m_availableDeviceTypes = types;
    }

    void setAddableDeviceTypes(const QList<IECDeviceType>& types)
    {
        m_addableDeviceTypes = types;
    }

    void setSupportedDeviceNumbers(IECDeviceType type, const QList<quint8>& numbers)
    {
        m_supportedDeviceNumbers.insert(type, numbers);
    }

private:
    QList<QSharedPointer<IECDeviceConfiguration>> m_configurations;
    QList<IECDeviceType> m_availableDeviceTypes;
    QList<IECDeviceType> m_addableDeviceTypes;
    QHash<IECDeviceType, QList<quint8>> m_supportedDeviceNumbers;
};


class IECSystemAddDeviceResponse : public IECSystemResponse
{
public:
    explicit IECSystemAddDeviceResponse(RequestId requestId)
        : IECSystemResponse(requestId)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }
    void setDeviceId(IECDeviceConfiguration::DeviceId deviceId) { m_deviceId = deviceId; }

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
};


class IECSystemRemoveDeviceResponse : public IECSystemResponse
{
public:
    explicit IECSystemRemoveDeviceResponse(RequestId requestId)
        : IECSystemResponse(requestId)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }
    void setDeviceId(IECDeviceConfiguration::DeviceId deviceId) { m_deviceId = deviceId; }

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
};


class IECSystemSetConfigurationResponse : public IECSystemResponse
{
public:
    explicit IECSystemSetConfigurationResponse(RequestId requestId)
        : IECSystemResponse(requestId)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }
    void setDeviceId(IECDeviceConfiguration::DeviceId deviceId) { m_deviceId = deviceId; }

    const QString& operationMessage() const { return m_operationMessage; }
    void setOperationMessage(const QString& message) { m_operationMessage = message; }

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
    QString m_operationMessage;
};


class IECSystemRestoreConfigurationsResponse : public IECSystemResponse
{
public:
    explicit IECSystemRestoreConfigurationsResponse(RequestId requestId)
        : IECSystemResponse(requestId)
    {
    }
};
