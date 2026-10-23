#pragma once

#include <QByteArray>
#include <QtGlobal>
#include <QList>
#include <QSharedPointer>

#include "EmulatorTypes.h"
#include "Input/InputEvent.h"
#include "C64/C64ROMSet.h"
#include "C64/IEC/IECDeviceConfiguration.h"


class EmulatorController;

class EmulatorRequest
{
public:
    using RequestId = quint64;

    enum class Type
    {
        LoadROMSet,
        LoadProgram,
        Input,
        PowerOn,
        PowerOff,
        Pause,
        SoftReset,
        HardReset,
        SetSpeed,
        Shutdown,
        GetIECConfigurations,
        AddIECDevice,
        RemoveIECDevice,
        SetIECConfiguration,
        RestoreIECConfigurations
    };

    explicit EmulatorRequest(Type type)
        : m_type(type)
    {
    }

    virtual ~EmulatorRequest() = default;

    EmulatorRequest(const EmulatorRequest&) = delete;
    EmulatorRequest& operator=(const EmulatorRequest&) = delete;

    Type type() const { return m_type; }
    RequestId id() const { return m_id; }

private:
    friend class EmulatorController;

    void setId(RequestId id) { m_id = id; }

    Type m_type;
    RequestId m_id = 0;
};

class EmulatorRequestLoadROMSet : public EmulatorRequest
{
public:
    EmulatorRequestLoadROMSet(const C64ROMSet& romSet)
        : EmulatorRequest(Type::LoadROMSet)
        , m_romSet(romSet)
    {
    }

    const C64ROMSet& romSet() const { return m_romSet; }

private:
    C64ROMSet m_romSet;
};

class EmulatorRequestLoadProgram : public EmulatorRequest
{
public:
    EmulatorRequestLoadProgram(quint16 address, const QByteArray& data)
        : EmulatorRequest(Type::LoadProgram)
        , m_address(address)
        , m_data(data)
    {
    }

    quint16 address() const { return m_address; }
    const QByteArray& data() const { return m_data; }

private:
    quint16 m_address;
    QByteArray m_data;
};

class EmulatorRequestInput : public EmulatorRequest
{
public:
    EmulatorRequestInput(const InputEvent& event)
        : EmulatorRequest(Type::Input)
        , m_event(event)
    {
    }

    const InputEvent& event() const { return m_event; }

private:
    InputEvent m_event;
};

class EmulatorRequestPowerOn : public EmulatorRequest
{
public:
    explicit EmulatorRequestPowerOn()
        : EmulatorRequest(Type::PowerOn)
    {
    }
};

class EmulatorRequestPowerOff : public EmulatorRequest
{
public:
    explicit EmulatorRequestPowerOff()
        : EmulatorRequest(Type::PowerOff)
    {
    }
};

class EmulatorRequestPause : public EmulatorRequest
{
public:
    EmulatorRequestPause(bool paused)
        : EmulatorRequest(Type::Pause)
        , m_paused(paused)
    {
    }

    bool paused() const { return m_paused; }

private:
    bool m_paused;
};

class EmulatorRequestSoftReset : public EmulatorRequest
{
public:
    explicit EmulatorRequestSoftReset()
        : EmulatorRequest(Type::SoftReset)
    {
    }
};

class EmulatorRequestHardReset : public EmulatorRequest
{
public:
    explicit EmulatorRequestHardReset()
        : EmulatorRequest(Type::HardReset)
    {
    }
};

class EmulatorRequestSetSpeed : public EmulatorRequest
{
public:
    EmulatorRequestSetSpeed(Emulator::Speed speed)
        : EmulatorRequest(Type::SetSpeed)
        , m_speed(speed)
    {
    }

    Emulator::Speed speed() const { return m_speed; }

private:
    Emulator::Speed m_speed;
};

class EmulatorRequestShutdown : public EmulatorRequest
{
public:
    explicit EmulatorRequestShutdown()
        : EmulatorRequest(Type::Shutdown)
    {
    }
};


class EmulatorRequestGetIECConfigurations : public EmulatorRequest
{
public:
    EmulatorRequestGetIECConfigurations()
        : EmulatorRequest(Type::GetIECConfigurations)
    {
    }
};


class EmulatorRequestAddIECDevice : public EmulatorRequest
{
public:
    explicit EmulatorRequestAddIECDevice(IECDeviceType type)
        : EmulatorRequest(Type::AddIECDevice)
        , m_type(type)
    {
    }

    IECDeviceType deviceType() const { return m_type; }

private:
    IECDeviceType m_type;
};

class EmulatorRequestRemoveIECDevice : public EmulatorRequest
{
public:
    explicit EmulatorRequestRemoveIECDevice(IECDeviceConfiguration::DeviceId id)
        : EmulatorRequest(Type::RemoveIECDevice)
        , m_deviceId(id)
    {
    }

    IECDeviceConfiguration::DeviceId deviceId() const { return m_deviceId; }

private:
    IECDeviceConfiguration::DeviceId m_deviceId;
};

class EmulatorRequestSetIECConfiguration : public EmulatorRequest
{
public:
    explicit EmulatorRequestSetIECConfiguration(const IECDeviceConfiguration& configuration)
        : EmulatorRequest(Type::SetIECConfiguration)
        , m_configuration(configuration.clone())
    {
    }

    const IECDeviceConfiguration& configuration() const { return *m_configuration; }

private:
    QSharedPointer<IECDeviceConfiguration> m_configuration;
};

class EmulatorRequestRestoreIECConfigurations : public EmulatorRequest
{
public:
    using ConfigurationPtr = QSharedPointer<IECDeviceConfiguration>;

    explicit EmulatorRequestRestoreIECConfigurations(const QList<ConfigurationPtr>& configurations)
        : EmulatorRequest(Type::RestoreIECConfigurations)
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

private:
    QList<ConfigurationPtr> m_configurations;
};
