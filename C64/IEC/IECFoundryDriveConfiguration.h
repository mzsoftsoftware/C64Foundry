#pragma once

#include "IECDeviceConfiguration.h"
#include "IECFoundryDiskConfiguration.h"

#include <QList>


class IECFoundryDriveConfiguration : public IECDeviceConfiguration
{
public:
    static constexpr int MaximumDiskCount = 4;

    IECFoundryDriveConfiguration();

    quint8 startDeviceNumber() const;
    int slotCount() const;
    int endDeviceNumber() const;

    void setStartDeviceNumber(quint8 deviceNumber);
    void setSlotCount(int slotCount);

    const QList<IECFoundryDiskConfiguration>& disks() const;

    IECFoundryDiskConfiguration* diskConfiguration(int slotIndex);
    const IECFoundryDiskConfiguration* diskConfiguration(int slotIndex) const;

    bool addDiskConfiguration(const IECFoundryDiskConfiguration& configuration);
    bool removeDiskConfiguration(int slotIndex);

    IECDeviceConfiguration* clone() const override;

protected:
    bool equals(const IECDeviceConfiguration& other) const override;

private:
    quint8 m_startDeviceNumber = 8;
    int m_slotCount = MaximumDiskCount;

    QList<IECFoundryDiskConfiguration> m_disks;
};
