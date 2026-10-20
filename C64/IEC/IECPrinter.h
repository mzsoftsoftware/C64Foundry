#pragma once

#include <QByteArray>

#include "IECDevice.h"


class IECPrinter : public IECDevice
{
public:
    explicit IECPrinter(quint8 deviceNumber, IECBus* ptrBus);
    virtual ~IECPrinter();

    bool hasOutput() const;
    quint64 outputRevision() const;
    QByteArray outputSnapshot() const;

    virtual void tearOff();
    void reset() override;

protected:
    void appendOutput(const QByteArray& data);
    virtual void clearPendingOutput();

private:
    QByteArray m_output;
    quint64 m_outputRevision = 0;
};
