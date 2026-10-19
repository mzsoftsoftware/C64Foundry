#pragma once

#include <QtGlobal>

#include "IECBus.h"


class IECDevice
{
public:
    explicit IECDevice(quint8 deviceNumber, IECBus* ptrBus);
    virtual ~IECDevice();

    quint8 deviceNumber() const                     { return m_deviceNumber; }

    virtual void reset() = 0;
    virtual void clock() = 0;

protected:
    bool lineHigh(IECBus::Line line) const;
    void setLineLow(IECBus::Line line, bool low);

    IECBus* iecBus() const                          { return m_ptrBus; }
    IECBus::ParticipantId participantId() const     { return m_participantId; }

private:
    quint8 m_deviceNumber = 0;
    IECBus* m_ptrBus = nullptr;
    IECBus::ParticipantId m_participantId = 0;
};
