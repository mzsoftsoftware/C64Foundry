#pragma once

#include <QByteArray>

#include "IECDevice.h"


class IECPrinter : public IECDevice
{
public:
    explicit IECPrinter(DeviceId id);
    virtual ~IECPrinter();

    virtual void tearOff();

    void reset() override;
    void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response) override;
    IECDeviceEventList takeEvents() override;

protected:
    void receiveByte(quint8 value);
    void finishTransmission();

    virtual void processReceivedData(const QByteArray& data);

private:
    QByteArray m_pendingOutput;

    bool m_listening = false;

    IECDeviceEventList m_events;
};
