#pragma once

#include "IECPrinter.h"


class IECAsciiPrinterConfiguration;


class IECAsciiPrinter : public IECPrinter
{
public:
    explicit IECAsciiPrinter(DeviceId id);
    virtual ~IECAsciiPrinter();

    bool setConfiguration(const IECAsciiPrinterConfiguration& configuration);

    void tearOff() override;
    void reset() override;

protected:
    void processReceivedData(const QByteArray& data) override;

private:
    int m_lineWidth = 80;
    bool m_autoWrap = true;
    int m_column = 0;
    bool m_lastWasCR = false;
};
