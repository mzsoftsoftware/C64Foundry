#include "IECFoundryDisk.h"

#include "IECFoundryDiskConfiguration.h"


IECFoundryDisk::IECFoundryDisk(IECBus* ptrBus, IECFoundryDrive* ptrDrive, quint8 deviceNumber)
    : IECDevice(ptrBus, deviceNumber)
    , m_ptrDrive(ptrDrive)
{
}

IECFoundryDisk::~IECFoundryDisk()
{
}

QList<quint8> IECFoundryDisk::supportedDeviceNumbers()
{
    return { 8, 9, 10, 11 };
}

bool IECFoundryDisk::setConfiguration(const IECFoundryDiskConfiguration& configuration)
{
    if (configuration.type() != IECDeviceType::FoundryDisk)
        return false;

    if (configuration.deviceNumber() != deviceNumber())
        return false;

    return true;
}

void IECFoundryDisk::reset()
{
}

void IECFoundryDisk::clock()
{
}
