#include "IECDevice.h"


IECDevice::IECDevice(const DeviceId id)
    : m_id(id)
{
}


IECDevice::~IECDevice() = default;


IECDevice::DeviceId IECDevice::id() const
{
    return m_id;
}
