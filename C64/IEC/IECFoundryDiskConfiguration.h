#pragma once

#include "IECDeviceConfiguration.h"

#include <QString>


class IECFoundryDiskConfiguration : public IECDeviceConfiguration
{
public:
    IECFoundryDiskConfiguration()
        : IECDeviceConfiguration(IECDeviceType::FoundryDisk)
    {
    }

    IECDeviceConfiguration* clone() const override
    {
        return new IECFoundryDiskConfiguration(*this);
    }

    QString imageFileName() const                   { return m_qstrImageFileName; }
    void setImageFileName(const QString& fileName)  { m_qstrImageFileName = fileName; }

    bool saveCurrentDiskOnExit() const              { return m_bSaveCurrentDiskOnExit; }
    void setSaveCurrentDiskOnExit(bool enabled)     { m_bSaveCurrentDiskOnExit = enabled; }

protected:
    bool equals(const IECDeviceConfiguration& other) const override
    {
        if (other.type() != IECDeviceType::FoundryDisk)
            return false;

        const IECFoundryDiskConfiguration* ptrOther = dynamic_cast<const IECFoundryDiskConfiguration*>(&other);

        if (ptrOther == nullptr)
            return false;

        if(m_qstrImageFileName != ptrOther->m_qstrImageFileName)
            return false;
        if(m_bSaveCurrentDiskOnExit != ptrOther->m_bSaveCurrentDiskOnExit)
            return false;

        return true;
    }

private:
    QString m_qstrImageFileName;
    bool m_bSaveCurrentDiskOnExit = false;
};
