#include "IECDeviceConfiguration.h"


IECDeviceConfiguration::IECDeviceConfiguration(const IECDeviceType type)
    : m_type(type)
{
}


IECDeviceConfiguration::DeviceId IECDeviceConfiguration::id() const
{
    return m_id;
}


IECDeviceType IECDeviceConfiguration::type() const
{
    return m_type;
}


bool IECDeviceConfiguration::enabled() const
{
    return m_enabled;
}


void IECDeviceConfiguration::setId(const DeviceId id)
{
    m_id = id;
}


void IECDeviceConfiguration::setEnabled(const bool enabled)
{
    m_enabled = enabled;
}


bool IECDeviceConfiguration::operator==(const IECDeviceConfiguration& other) const
{
    return m_id == other.m_id &&
           m_type == other.m_type &&
           m_enabled == other.m_enabled &&
           equals(other);
}


bool IECDeviceConfiguration::operator!=(const IECDeviceConfiguration& other) const
{
    return !(*this == other);
}
