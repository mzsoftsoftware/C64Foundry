#pragma once

#include <QtGlobal>
#include <QSharedPointer>


enum class IECDeviceType
{
    PrinterASCII,
    FoundryDisk
};


class IECDeviceConfiguration
{
public:
    using DeviceId = quint64;

    explicit IECDeviceConfiguration(IECDeviceType type)
        : m_type(type)
    {
    }

    virtual ~IECDeviceConfiguration() = default;

    IECDeviceConfiguration(const IECDeviceConfiguration&) = default;
    IECDeviceConfiguration& operator=(const IECDeviceConfiguration&) = default;

    DeviceId id() const { return m_id; }
    IECDeviceType type() const { return m_type; }
    quint8 deviceNumber() const { return m_deviceNumber; }
    bool enabled() const { return m_enabled; }

    void setId(DeviceId id) { m_id = id; }
    void setDeviceNumber(quint8 deviceNumber) { m_deviceNumber = deviceNumber; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    virtual IECDeviceConfiguration* clone() const = 0;

    bool operator==(const IECDeviceConfiguration& other) const
    {
        return m_id == other.m_id &&
               m_type == other.m_type &&
               m_deviceNumber == other.m_deviceNumber &&
               m_enabled == other.m_enabled &&
               equals(other);
    }

    bool operator!=(const IECDeviceConfiguration& other) const
    {
        return !(*this == other);
    }

protected:
    virtual bool equals(const IECDeviceConfiguration& other) const = 0;

private:
    DeviceId m_id = 0;
    IECDeviceType m_type;
    quint8 m_deviceNumber = 4;
    bool m_enabled = false;
};


//using IECDeviceConfigurationPtr = QSharedPointer<IECDeviceConfiguration>;
