#include "IECDeviceManager.h"

#include "IECDevice.h"


bool IECDeviceManager::registerDevice(quint8 deviceNumber, IECDevice* ptrDevice)
{
    if (deviceNumber > MAX_DEVICE_NUMBER || ptrDevice == nullptr)
        return false;

    if (m_devices[deviceNumber] != nullptr)
        return false;

    m_devices[deviceNumber] = ptrDevice;

    return true;
}


bool IECDeviceManager::unregisterDevice(quint8 deviceNumber, IECDevice* ptrDevice)
{
    if (deviceNumber > MAX_DEVICE_NUMBER || ptrDevice == nullptr)
        return false;

    if (m_devices[deviceNumber] != ptrDevice)
        return false;

    m_devices[deviceNumber] = nullptr;

    return true;
}


IECDevice* IECDeviceManager::device(quint8 deviceNumber) const
{
    if (deviceNumber > MAX_DEVICE_NUMBER)
        return nullptr;

    return m_devices[deviceNumber];
}
