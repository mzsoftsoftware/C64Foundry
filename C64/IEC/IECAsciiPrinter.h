#pragma once

#include "IECPrinter.h"
#include "IECProtocol.h"

#include <QList>


class IECAsciiPrinter : public IECPrinter
{
public:
    explicit IECAsciiPrinter(IECBus* ptrBus, quint8 deviceNumber = 4);
    virtual ~IECAsciiPrinter();

    QList<quint8> supportedDeviceNumbers() const override;

    void reset() override;
    void clock() override;

private:
    IECProtocol m_protocol;
};
