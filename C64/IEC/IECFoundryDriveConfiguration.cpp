#include "IECFoundryDriveConfiguration.h"


IECFoundryDriveConfiguration::IECFoundryDriveConfiguration()
    : IECDeviceConfiguration(IECDeviceType::FoundryDrive)
{
}


quint8 IECFoundryDriveConfiguration::startDeviceNumber() const
{
    return m_startDeviceNumber;
}
int IECFoundryDriveConfiguration::slotCount() const
{
    return m_disks.size();
}
int IECFoundryDriveConfiguration::endDeviceNumber() const
{
    return static_cast<int>(m_startDeviceNumber) + slotCount() - 1;
}


void IECFoundryDriveConfiguration::setStartDeviceNumber(const quint8 deviceNumber)
{
    m_startDeviceNumber = deviceNumber;
}
const QList<IECFoundryDiskConfiguration>& IECFoundryDriveConfiguration::disks() const
{
    return m_disks;
}
IECFoundryDiskConfiguration* IECFoundryDriveConfiguration::diskConfiguration(const int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= m_disks.size())
        return nullptr;

    return &m_disks[slotIndex];
}
const IECFoundryDiskConfiguration* IECFoundryDriveConfiguration::diskConfiguration(const int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= m_disks.size())
        return nullptr;

    return &m_disks[slotIndex];
}

bool IECFoundryDriveConfiguration::addDiskConfiguration(const IECFoundryDiskConfiguration& configuration)
{
    if (m_disks.size() >= MaximumDiskCount)
        return false;

    if (configuration.slotIndex() != m_disks.size())
        return false;

    m_disks.append(configuration);

    return true;
}

IECDeviceConfiguration* IECFoundryDriveConfiguration::clone() const
{
    return new IECFoundryDriveConfiguration(*this);
}

bool IECFoundryDriveConfiguration::equals(const IECDeviceConfiguration& other) const
{
    const IECFoundryDriveConfiguration* ptrOther = dynamic_cast<const IECFoundryDriveConfiguration*>(&other);
    if (ptrOther == nullptr)
        return false;

    if (m_startDeviceNumber != ptrOther->m_startDeviceNumber ||
        m_disks.size() != ptrOther->m_disks.size())
    {
        return false;
    }

    for (const IECFoundryDiskConfiguration& configuration : m_disks)
    {
        const IECFoundryDiskConfiguration* ptrOtherDisk = ptrOther->diskConfiguration(configuration.slotIndex());
        if (ptrOtherDisk == nullptr || configuration != *ptrOtherDisk)
            return false;
    }

    return true;
}
