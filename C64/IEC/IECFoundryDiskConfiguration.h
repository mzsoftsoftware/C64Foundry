#pragma once

#include "IECDeviceConfiguration.h"


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

protected:
    bool equals(const IECDeviceConfiguration& other) const override
    {
        if (other.type() != IECDeviceType::FoundryDisk)
            return false;

        const IECFoundryDiskConfiguration* ptrOther = dynamic_cast<const IECFoundryDiskConfiguration*>(&other);

        if (ptrOther == nullptr)
            return false;

        return true;
    }

private:
};
