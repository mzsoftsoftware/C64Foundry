#pragma once

#include "IECPrinter.h"

#include <QByteArray>
#include <QList>

#include "IECProtocol.h"


class IECAsciiPrinter : public IECPrinter
{
public:
    explicit IECAsciiPrinter(IECBus* ptrBus, quint8 deviceNumber = 4);
    virtual ~IECAsciiPrinter();

    QList<quint8> supportedDeviceNumbers() const override;

    void reset() override;
    void clock() override;

protected:
    void clearPendingOutput() override;

private:
    IECProtocol m_protocol;
    QByteArray m_pendingOutput;
};
