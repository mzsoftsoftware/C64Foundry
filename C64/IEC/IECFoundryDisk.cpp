#include "IECFoundryDisk.h"

#include "IECFoundryDiskConfiguration.h"
#include "IECFoundryDrive.h"


QList<quint8> IECFoundryDisk::supportedDeviceNumbers()
{
    return { 8, 9, 10, 11 };
}

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

bool IECFoundryDisk::setConfiguration(const IECFoundryDiskConfiguration& configuration, QString* errorMessage)
{
    Q_ASSERT(m_ptrDrive);

    if (configuration.type() != IECDeviceType::FoundryDisk)
        return false;

    if (configuration.deviceNumber() != deviceNumber())
        return false;

    return m_ptrDrive->insertImage(deviceNumber(), configuration.imageFileName(), errorMessage);
}

bool IECFoundryDisk::hasImage() const
{
    return m_ptrDrive != nullptr && m_ptrDrive->hasImage(deviceNumber());
}

void IECFoundryDisk::reset()
{
}

void IECFoundryDisk::clock()
{
}
