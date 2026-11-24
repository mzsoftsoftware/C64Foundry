#include "IECDeviceManager.h"

#include "IECDevice.h"
#include "IECProtocolEvent.h"


bool IECDeviceManager::registerDevice(const quint8 deviceNumber, IECDevice* ptrDevice)
{
    if (deviceNumber > MAX_DEVICE_NUMBER || ptrDevice == nullptr)
        return false;

    if (m_devices[deviceNumber] != nullptr)
        return false;

    m_devices[deviceNumber] = ptrDevice;

    return true;
}


bool IECDeviceManager::unregisterDevice(const quint8 deviceNumber, IECDevice* ptrDevice)
{
    if (deviceNumber > MAX_DEVICE_NUMBER || ptrDevice == nullptr)
        return false;

    if (m_devices[deviceNumber] != ptrDevice)
        return false;

    m_devices[deviceNumber] = nullptr;

    return true;
}


IECDevice* IECDeviceManager::device(const quint8 deviceNumber) const
{
    if (deviceNumber > MAX_DEVICE_NUMBER)
        return nullptr;

    return m_devices[deviceNumber];
}


void IECDeviceManager::processEvent(const IECProtocolEvent& event, IECProtocolResponse& response)
{
    IECDevice* ptrDevice = device(event.deviceNumber);

    if (ptrDevice == nullptr)
        return;

    ptrDevice->processEvent(event, response);
}


void IECDeviceManager::reset()
{
    IECDevice* processedDevices[DEVICE_NUMBER_COUNT] = {};
    int processedCount = 0;

    for (quint8 deviceNumber = 0; deviceNumber <= MAX_DEVICE_NUMBER; ++deviceNumber)
    {
        IECDevice* ptrDevice = m_devices[deviceNumber];

        if (ptrDevice == nullptr)
            continue;

        bool alreadyProcessed = false;

        for (int i = 0; i < processedCount; ++i)
        {
            if (processedDevices[i] == ptrDevice)
            {
                alreadyProcessed = true;
                break;
            }
        }

        if (alreadyProcessed)
            continue;

        processedDevices[processedCount++] = ptrDevice;
        ptrDevice->reset();
    }
}


IECDeviceEventList IECDeviceManager::takeEvents()
{
    IECDeviceEventList events;

    IECDevice* processedDevices[DEVICE_NUMBER_COUNT] = {};
    int processedCount = 0;

    for (quint8 deviceNumber = 0; deviceNumber <= MAX_DEVICE_NUMBER; ++deviceNumber)
    {
        IECDevice* ptrDevice = m_devices[deviceNumber];

        if (ptrDevice == nullptr)
            continue;

        bool alreadyProcessed = false;

        for (int i = 0; i < processedCount; ++i)
        {
            if (processedDevices[i] == ptrDevice)
            {
                alreadyProcessed = true;
                break;
            }
        }

        if (alreadyProcessed)
            continue;

        processedDevices[processedCount++] = ptrDevice;

        events.append(ptrDevice->takeEvents());
    }

    return events;
}
