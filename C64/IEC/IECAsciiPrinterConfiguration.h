#pragma once

#include "IECDeviceConfiguration.h"


class IECAsciiPrinterConfiguration : public IECDeviceConfiguration
{
public:
    IECAsciiPrinterConfiguration()
        : IECDeviceConfiguration(IECDeviceType::PrinterASCII)
    {
    }

    IECDeviceConfiguration* clone() const override
    {
        return new IECAsciiPrinterConfiguration(*this);
    }

protected:
    bool equals(const IECDeviceConfiguration& other) const override
    {
        if (other.type() != IECDeviceType::PrinterASCII)
            return false;

        // Der ASCII-Drucker besitzt aktuell noch keine
        // zusätzlichen Konfigurationseigenschaften.
        return true;
    }
};
