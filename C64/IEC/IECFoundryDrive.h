#pragma once

#include <QByteArray>
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

    bool setDiskEnabled(quint8 deviceNumber, bool enabled);
    bool insertImage(quint8 deviceNumber, const QString& fileName);
    bool ejectImage(quint8 deviceNumber);
    bool saveImage(quint8 deviceNumber);

    void reset() override;
    void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response) override;
    IECDeviceEventList takeEvents() override;

private:
    struct ChannelState
    {
        bool opened = false;
        bool readingFile = false;
        QByteArray fileName;
        QByteArray inputBuffer;
        quint8 currentTrack = 0;
        quint8 currentSector = 0;
    };
    struct DiskChannelState
    {
        QList<ChannelState> channels;
    };

    IECFoundryDisk* findDisk(quint8 deviceNumber) const;
    ChannelState* findChannelState(quint8 deviceNumber, quint8 channel);
    void collectDiskEvents();
    bool prepareNextFileBlock(IECFoundryDisk* ptrDisk, ChannelState& channelState, IECProtocolResponse& response);

private:
    IECFoundryDriveConfiguration m_configuration;
    QList<IECFoundryDisk*> m_disks;
    QList<DiskChannelState> m_channelStates;
    IECDeviceEventList m_events;
    DiskId m_nextDiskId = 1;
};
