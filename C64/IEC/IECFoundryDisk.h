#pragma once

#include "IECDevice.h"

class IECFoundryDiskConfiguration;


class IECFoundryDisk : public IECDevice
{
public:
    static QList<quint8> supportedDeviceNumbers();

    explicit IECFoundryDisk(IECBus* ptrBus, quint8 deviceNumber = 8);
    virtual ~IECFoundryDisk();

    void reset() override;
    void clock() override;

    bool setConfiguration(const IECFoundryDiskConfiguration& configuration);
};
