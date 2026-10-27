#pragma once

#include "IECDevice.h"

class IECFoundryDiskConfiguration;


class IECFoundryDisk : public IECDevice
{
public:
    explicit IECFoundryDisk(IECBus* ptrBus, quint8 deviceNumber = 8);
    virtual ~IECFoundryDisk();

    QList<quint8> supportedDeviceNumbers() const override;

    void reset() override;
    void clock() override;

    bool setConfiguration(const IECFoundryDiskConfiguration& configuration);
};
