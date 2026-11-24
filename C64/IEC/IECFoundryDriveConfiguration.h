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

    const QList<IECFoundryDiskConfiguration>& disks() const;
    IECFoundryDiskConfiguration* diskConfiguration(int slotIndex);
    const IECFoundryDiskConfiguration* diskConfiguration(int slotIndex) const;

    bool addDiskConfiguration(const IECFoundryDiskConfiguration& configuration);

    IECDeviceConfiguration* clone() const override;

protected:
    bool equals(const IECDeviceConfiguration& other) const override;

private:
    quint8 m_startDeviceNumber = 8;

    QList<IECFoundryDiskConfiguration> m_disks;
};
