#pragma once

#include "IECDeviceConfiguration.h"


class IECAsciiPrinterConfiguration : public IECDeviceConfiguration
{
public:
    IECAsciiPrinterConfiguration()
        : IECDeviceConfiguration(IECDeviceType::PrinterASCII)
    {
    }

    int lineWidth() const { return m_lineWidth; }
    bool autoWrap() const { return m_autoWrap; }

    void setLineWidth(int lineWidth) { m_lineWidth = lineWidth; }
    void setAutoWrap(bool autoWrap) { m_autoWrap = autoWrap; }

    IECDeviceConfiguration* clone() const override
    {
        return new IECAsciiPrinterConfiguration(*this);
    }

protected:
    bool equals(const IECDeviceConfiguration& other) const override
    {
        if (other.type() != IECDeviceType::PrinterASCII)
            return false;

        const IECAsciiPrinterConfiguration* ptrOther = dynamic_cast<const IECAsciiPrinterConfiguration*>(&other);

        if (ptrOther == nullptr)
            return false;

        return m_lineWidth == ptrOther->m_lineWidth &&
               m_autoWrap == ptrOther->m_autoWrap;
    }

private:
    int m_lineWidth = 80;
    bool m_autoWrap = true;
};
