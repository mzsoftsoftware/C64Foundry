#pragma once

#include "IECPrinter.h"
#include "IECProtocol.h"

#include <QList>

class IECAsciiPrinterConfiguration;


class IECAsciiPrinter : public IECPrinter
{
public:
    explicit IECAsciiPrinter(IECBus* ptrBus, quint8 deviceNumber = 4);
    virtual ~IECAsciiPrinter();

    QList<quint8> supportedDeviceNumbers() const override;

    bool setConfiguration(const IECAsciiPrinterConfiguration& configuration);

    void tearOff() override;
    void reset() override;
    void clock() override;

protected:
    void processReceivedData(const QByteArray& data) override;

private:
    IECProtocol m_protocol;

    int m_lineWidth = 80;
    bool m_autoWrap = true;
    int m_column = 0;
    bool m_lastWasCR = false;
};
