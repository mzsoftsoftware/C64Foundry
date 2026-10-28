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


bool IECFoundryDisk::setConfiguration(const IECFoundryDiskConfiguration& configuration)
{
    Q_ASSERT(m_ptrDrive);

    if (configuration.type() != IECDeviceType::FoundryDisk)
        return false;
    if (configuration.deviceNumber() != deviceNumber())
        return false;

    const QString fileName = configuration.imageFileName();
    if (fileName.isEmpty())
    {
        if (m_ptrDrive->hasImage(deviceNumber()))
            return m_ptrDrive->ejectImage(deviceNumber());

        return true;
    }

    return m_ptrDrive->insertImage(deviceNumber(), fileName);
}

void IECFoundryDisk::reset()
{
}

void IECFoundryDisk::clock()
{
}
