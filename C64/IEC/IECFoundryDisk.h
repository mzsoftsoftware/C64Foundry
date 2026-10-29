#pragma once

#include "IECDevice.h"
#include "IECProtocol.h"

class IECFoundryDiskConfiguration;
class IECFoundryDrive;


class IECFoundryDisk : public IECDevice
{
public:
    static QList<quint8> supportedDeviceNumbers();

    explicit IECFoundryDisk(IECBus* ptrBus, IECFoundryDrive* ptrDrive, quint8 deviceNumber = 8);
    virtual ~IECFoundryDisk();

    bool setConfiguration(const IECFoundryDiskConfiguration& configuration, QString* errorMessage = nullptr);
    bool hasImage() const;

    void reset() override;
    void clock() override;

private:
    void processReceivedBytes();
    void finishListen();
    void startRequestedTransfer();

private:
    IECFoundryDrive* m_ptrDrive = nullptr;
    IECProtocol m_protocol;

    QByteArray m_listenFileName;
    QByteArray m_requestedFileName;

    bool m_pendingTalk = false;
};
