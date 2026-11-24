#include "IECFoundryDiskConfiguration.h"


IECFoundryDiskConfiguration::DiskId IECFoundryDiskConfiguration::diskId() const
{
    return m_diskId;
}
int IECFoundryDiskConfiguration::slotIndex() const
{
    return m_slotIndex;
}
quint8 IECFoundryDiskConfiguration::deviceNumber() const
{
    return m_deviceNumber;
}
bool IECFoundryDiskConfiguration::enabled() const
{
    return m_enabled;
}
QString IECFoundryDiskConfiguration::imageFileName() const
{
    return m_imageFileName;
}
bool IECFoundryDiskConfiguration::saveCurrentDiskOnExit() const
{
    return m_saveCurrentDiskOnExit;
}

void IECFoundryDiskConfiguration::setDiskId(const DiskId diskId)
{
    m_diskId = diskId;
}
void IECFoundryDiskConfiguration::setSlotIndex(const int slotIndex)
{
    m_slotIndex = slotIndex;
}
void IECFoundryDiskConfiguration::setDeviceNumber(const quint8 deviceNumber)
{
    m_deviceNumber = deviceNumber;
}
void IECFoundryDiskConfiguration::setEnabled(const bool enabled)
{
    m_enabled = enabled;
}
void IECFoundryDiskConfiguration::setImageFileName(const QString& fileName)
{
    m_imageFileName = fileName;
}
void IECFoundryDiskConfiguration::setSaveCurrentDiskOnExit(const bool enabled)
{
    m_saveCurrentDiskOnExit = enabled;
}

bool IECFoundryDiskConfiguration::operator==(const IECFoundryDiskConfiguration& other) const
{
    return m_slotIndex == other.m_slotIndex &&
           m_deviceNumber == other.m_deviceNumber &&
           m_enabled == other.m_enabled &&
           m_imageFileName == other.m_imageFileName &&
           m_saveCurrentDiskOnExit == other.m_saveCurrentDiskOnExit;
}


bool IECFoundryDiskConfiguration::operator!=(const IECFoundryDiskConfiguration& other) const
{
    return !(*this == other);
}
