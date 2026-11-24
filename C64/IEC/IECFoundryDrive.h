#pragma once

#include <QList>
#include <QString>

#include "IECDevice.h"
#include "IECFoundryDriveConfiguration.h"


class IECFoundryDisk;


class IECFoundryDrive : public IECDevice
{
public:
    using DiskId = IECFoundryDiskConfiguration::DiskId;

    explicit IECFoundryDrive(DeviceId id);
    virtual ~IECFoundryDrive();

    IECFoundryDrive(const IECFoundryDrive&) = delete;
    IECFoundryDrive& operator=(const IECFoundryDrive&) = delete;

    bool setConfiguration(const IECFoundryDriveConfiguration& configuration);
    IECFoundryDriveConfiguration configuration() const;

    int diskCount() const;
    IECFoundryDisk* disk(int slotIndex) const;
    IECFoundryDisk* diskByDeviceNumber(quint8 deviceNumber) const;

    bool insertImage(quint8 deviceNumber, const QString& fileName);
    bool ejectImage(quint8 deviceNumber);
    bool saveImage(quint8 deviceNumber);

    void reset() override;
    void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response) override;
    IECDeviceEventList takeEvents() override;

private:
    IECFoundryDisk* findDisk(quint8 deviceNumber) const;
    void collectDiskEvents();

private:
    IECFoundryDriveConfiguration m_configuration;
    QList<IECFoundryDisk*> m_disks;
    IECDeviceEventList m_events;
    DiskId m_nextDiskId = 1;
};
