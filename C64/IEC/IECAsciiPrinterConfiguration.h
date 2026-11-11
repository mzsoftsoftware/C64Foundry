#pragma once

#include "IECDeviceConfiguration.h"


class IECAsciiPrinterConfiguration : public IECDeviceConfiguration
{
public:
    IECAsciiPrinterConfiguration();

    quint8 deviceNumber() const;
    int lineWidth() const;
    bool autoWrap() const;

    void setDeviceNumber(quint8 deviceNumber);
    void setLineWidth(int lineWidth);
    void setAutoWrap(bool autoWrap);

    IECDeviceConfiguration* clone() const override;

protected:
    bool equals(const IECDeviceConfiguration& other) const override;

private:
    quint8 m_deviceNumber = 4;
    int m_lineWidth = 80;
    bool m_autoWrap = true;
};
