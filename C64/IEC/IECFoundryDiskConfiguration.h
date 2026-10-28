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

    QString imageFileName() const { return m_imageFileName; }
    void setImageFileName(const QString& fileName) { m_imageFileName = fileName; }

protected:
    bool equals(const IECDeviceConfiguration& other) const override
    {
        if (other.type() != IECDeviceType::FoundryDisk)
            return false;

        const IECFoundryDiskConfiguration* ptrOther = dynamic_cast<const IECFoundryDiskConfiguration*>(&other);

        if (ptrOther == nullptr)
            return false;

        return m_imageFileName == ptrOther->m_imageFileName;
    }

private:
    QString m_imageFileName;
};
