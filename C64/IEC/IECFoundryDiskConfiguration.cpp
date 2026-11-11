#include "IECFoundryDiskConfiguration.h"


int IECFoundryDiskConfiguration::slotIndex() const
{
    return m_slotIndex;
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


void IECFoundryDiskConfiguration::setSlotIndex(const int slotIndex)
{
    m_slotIndex = slotIndex;
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
           m_enabled == other.m_enabled &&
           m_imageFileName == other.m_imageFileName &&
           m_saveCurrentDiskOnExit == other.m_saveCurrentDiskOnExit;
}


bool IECFoundryDiskConfiguration::operator!=(const IECFoundryDiskConfiguration& other) const
{
    return !(*this == other);
}
