#pragma once

#include <QByteArray>

#include "IECDevice.h"


class IECPrinter : public IECDevice
{
public:
    explicit IECPrinter(DeviceId id);
    virtual ~IECPrinter();

    bool hasOutput() const;
    quint64 outputRevision() const;
    QByteArray outputSnapshot() const;

    virtual void tearOff();

    void reset() override;
    void processEvent(const IECProtocolEvent& event, IECProtocolResponse& response) override;
    IECDeviceEventList takeEvents() override;

protected:
    void receiveByte(quint8 value);
    void finishTransmission();

    virtual void processReceivedData(const QByteArray& data);

private:
    QByteArray m_output;
    QByteArray m_pendingOutput;
    quint64 m_outputRevision = 0;

    bool m_listening = false;

    IECDeviceEventList m_events;
};
