#pragma once

#include <QtGlobal>


enum class IECDeviceType
{
    PrinterASCII,
    FoundryDrive
};


class IECDeviceConfiguration
{
public:
    using DeviceId = quint64;

    explicit IECDeviceConfiguration(IECDeviceType type);
    virtual ~IECDeviceConfiguration() = default;

    IECDeviceConfiguration(const IECDeviceConfiguration&) = default;
    IECDeviceConfiguration& operator=(const IECDeviceConfiguration&) = default;

    DeviceId id() const;
    IECDeviceType type() const;
    bool enabled() const;

    void setId(DeviceId id);
    void setEnabled(bool enabled);

    virtual IECDeviceConfiguration* clone() const = 0;

    bool operator==(const IECDeviceConfiguration& other) const;
    bool operator!=(const IECDeviceConfiguration& other) const;

protected:
    virtual bool equals(const IECDeviceConfiguration& other) const = 0;

private:
    DeviceId m_id = 0;
    IECDeviceType m_type;
    bool m_enabled = false;
};
