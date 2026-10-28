#include "IECFoundryDisk.h"

#include "IECFoundryDiskConfiguration.h"
#include "IECFoundryDrive.h"


IECFoundryDisk::IECFoundryDisk(IECBus* ptrBus, IECFoundryDrive* ptrDrive, quint8 deviceNumber)
    : IECDevice(ptrBus, deviceNumber)
    , m_ptrDrive(ptrDrive)
{
    m_ptrDrive->registerDisk(this);
}
IECFoundryDisk::~IECFoundryDisk()
{
    m_ptrDrive->unregisterDisk(this);
}

QList<quint8> IECFoundryDisk::supportedDeviceNumbers()
{
    return { 8, 9, 10, 11 };
}


bool IECFoundryDisk::setConfiguration(const IECFoundryDiskConfiguration& configuration, QString* errorMessage)
{
    Q_ASSERT(m_ptrDrive);

    if (configuration.type() != IECDeviceType::FoundryDisk)
        return false;

    if (configuration.deviceNumber() != deviceNumber())
        return false;

    return m_ptrDrive->insertImage(deviceNumber(), configuration.imageFileName(), errorMessage);
}

void IECFoundryDisk::reset()
{
}

void IECFoundryDisk::clock()
{
}
