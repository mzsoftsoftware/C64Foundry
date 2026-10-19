#pragma once

#include <QByteArray>

#include "IECDevice.h"
#include "IECProtocol.h"


class IECAsciiPrinter : public IECDevice
{
public:
    explicit IECAsciiPrinter(IECBus* ptrBus, quint8 deviceNumber = 4);
    virtual ~IECAsciiPrinter();

    void reset() override;
    void clock() override;

    bool hasOutput() const;
    QByteArray takeOutput();
    void clearOutput();

private:
    IECProtocol m_protocol;
    QByteArray m_output;
};
