#pragma once

#include <QtGlobal>
#include <QList>
#include <QHash>
#include <QSharedPointer>

#include "EmulatorRequest.h"
#include "C64/IEC/IECDeviceConfiguration.h"


class EmulatorResponse
{
public:
    enum class Result
    {
        Success,
        Failed,
        InvalidState,
        InvalidParameter,
        Cancelled
    };

    EmulatorResponse(EmulatorRequest::RequestId id, EmulatorRequest::Type type, Result result)
        : m_id(id)
        , m_type(type)
        , m_result(result)
    {
    }

    virtual ~EmulatorResponse() = default;

    EmulatorRequest::RequestId id() const { return m_id; }
    EmulatorRequest::Type type() const { return m_type; }
    Result result() const { return m_result; }

    bool success() const { return m_result == Result::Success; }

protected:
    void setResult(Result result) { m_result = result; }

private:
    EmulatorRequest::RequestId m_id;
    EmulatorRequest::Type m_type;
    Result m_result;
};

class EmulatorResponseIEC : public EmulatorResponse
{
public:
    using ConfigurationPtr = QSharedPointer<IECDeviceConfiguration>;

    EmulatorResponseIEC(EmulatorRequest::RequestId id, EmulatorRequest::Type type, Result result)
        : EmulatorResponse(id, type, result)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }
    const QList<ConfigurationPtr>& configurations() const { return m_configurations; }

    const QList<IECDeviceType>& availableDeviceTypes() const { return m_availableDeviceTypes; }
    const QList<IECDeviceType>& addableDeviceTypes() const { return m_addableDeviceTypes; }

    void setAvailableDeviceTypes(const QList<IECDeviceType>& types)                     { m_availableDeviceTypes = types; }
    void setAddableDeviceTypes(const QList<IECDeviceType>& types)                       { m_addableDeviceTypes = types; }
    void setDeviceId(IECDeviceConfiguration::DeviceId id)                               { m_deviceId = id; }

    void setConfigurations(const QList<ConfigurationPtr>& configurations)
    {
        m_configurations.clear();

        for (const ConfigurationPtr& ptrConfiguration : configurations)
        {
            if (!ptrConfiguration.isNull())
                m_configurations.append(ConfigurationPtr(ptrConfiguration->clone()));
        }
    }
    void setResult(Result result)                                                       { EmulatorResponse::setResult(result); }
    const QHash<IECDeviceType, QList<quint8>>& supportedDeviceNumbers() const           { return m_supportedDeviceNumbers; }
    void setSupportedDeviceNumbers(IECDeviceType type, const QList<quint8>& numbers)    { m_supportedDeviceNumbers.insert(type, numbers); }

private:
    IECDeviceConfiguration::DeviceId m_deviceId = 0;
    QList<ConfigurationPtr> m_configurations;
    QList<IECDeviceType> m_availableDeviceTypes;
    QList<IECDeviceType> m_addableDeviceTypes;
    QHash<IECDeviceType, QList<quint8>> m_supportedDeviceNumbers;
};
