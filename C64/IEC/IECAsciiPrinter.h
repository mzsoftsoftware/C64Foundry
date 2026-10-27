#pragma once

#include "IECPrinter.h"
#include "IECProtocol.h"

#include <QList>

class IECAsciiPrinterConfiguration;


class IECAsciiPrinter : public IECPrinter
{
public:
    static QList<quint8> supportedDeviceNumbers();

    explicit IECAsciiPrinter(IECBus* ptrBus, quint8 deviceNumber = 4);
    virtual ~IECAsciiPrinter();

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
