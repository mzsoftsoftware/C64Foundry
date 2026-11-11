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
    return m_slotCount;
}


int IECFoundryDriveConfiguration::endDeviceNumber() const
{
    return static_cast<int>(m_startDeviceNumber) + m_slotCount - 1;
}


void IECFoundryDriveConfiguration::setStartDeviceNumber(const quint8 deviceNumber)
{
    m_startDeviceNumber = deviceNumber;
}


void IECFoundryDriveConfiguration::setSlotCount(const int slotCount)
{
    m_slotCount = slotCount;
}


const QList<IECFoundryDiskConfiguration>& IECFoundryDriveConfiguration::disks() const
{
    return m_disks;
}


IECFoundryDiskConfiguration* IECFoundryDriveConfiguration::diskConfiguration(const int slotIndex)
{
    for (IECFoundryDiskConfiguration& configuration : m_disks)
    {
        if (configuration.slotIndex() == slotIndex)
            return &configuration;
    }

    return nullptr;
}


const IECFoundryDiskConfiguration* IECFoundryDriveConfiguration::diskConfiguration(const int slotIndex) const
{
    for (const IECFoundryDiskConfiguration& configuration : m_disks)
    {
        if (configuration.slotIndex() == slotIndex)
            return &configuration;
    }

    return nullptr;
}


bool IECFoundryDriveConfiguration::addDiskConfiguration(const IECFoundryDiskConfiguration& configuration)
{
    if (configuration.slotIndex() < 0 || configuration.slotIndex() >= m_slotCount)
        return false;

    if (diskConfiguration(configuration.slotIndex()) != nullptr)
        return false;

    m_disks.append(configuration);

    return true;
}


bool IECFoundryDriveConfiguration::removeDiskConfiguration(const int slotIndex)
{
    for (int index = 0; index < m_disks.size(); ++index)
    {
        if (m_disks.at(index).slotIndex() == slotIndex)
        {
            m_disks.removeAt(index);
            return true;
        }
    }

    return false;
}


IECDeviceConfiguration* IECFoundryDriveConfiguration::clone() const
{
    return new IECFoundryDriveConfiguration(*this);
}


bool IECFoundryDriveConfiguration::equals(const IECDeviceConfiguration& other) const
{
    const IECFoundryDriveConfiguration* ptrOther =
        dynamic_cast<const IECFoundryDriveConfiguration*>(&other);

    if (ptrOther == nullptr)
        return false;

    if (m_startDeviceNumber != ptrOther->m_startDeviceNumber ||
        m_slotCount != ptrOther->m_slotCount ||
        m_disks.size() != ptrOther->m_disks.size())
    {
        return false;
    }

    for (const IECFoundryDiskConfiguration& configuration : m_disks)
    {
        const IECFoundryDiskConfiguration* ptrOtherDisk =
            ptrOther->diskConfiguration(configuration.slotIndex());

        if (ptrOtherDisk == nullptr || configuration != *ptrOtherDisk)
            return false;
    }

    return true;
}
