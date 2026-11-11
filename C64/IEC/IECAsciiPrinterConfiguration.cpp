#include "IECAsciiPrinterConfiguration.h"


IECAsciiPrinterConfiguration::IECAsciiPrinterConfiguration()
    : IECDeviceConfiguration(IECDeviceType::PrinterASCII)
{
}


quint8 IECAsciiPrinterConfiguration::deviceNumber() const
{
    return m_deviceNumber;
}


int IECAsciiPrinterConfiguration::lineWidth() const
{
    return m_lineWidth;
}


bool IECAsciiPrinterConfiguration::autoWrap() const
{
    return m_autoWrap;
}


void IECAsciiPrinterConfiguration::setDeviceNumber(const quint8 deviceNumber)
{
    m_deviceNumber = deviceNumber;
}


void IECAsciiPrinterConfiguration::setLineWidth(const int lineWidth)
{
    m_lineWidth = lineWidth;
}


void IECAsciiPrinterConfiguration::setAutoWrap(const bool autoWrap)
{
    m_autoWrap = autoWrap;
}


IECDeviceConfiguration* IECAsciiPrinterConfiguration::clone() const
{
    return new IECAsciiPrinterConfiguration(*this);
}


bool IECAsciiPrinterConfiguration::equals(const IECDeviceConfiguration& other) const
{
    const IECAsciiPrinterConfiguration* ptrOther =
        dynamic_cast<const IECAsciiPrinterConfiguration*>(&other);

    if (ptrOther == nullptr)
        return false;

    return m_deviceNumber == ptrOther->m_deviceNumber &&
           m_lineWidth == ptrOther->m_lineWidth &&
           m_autoWrap == ptrOther->m_autoWrap;
}
