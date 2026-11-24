#include "IECDeviceFactory.h"


IECDeviceFactory::IECDeviceFactory()
{
}
IECDeviceFactory::~IECDeviceFactory()
{
}

QList<quint8> IECDeviceFactory::supportedDeviceNumbers(const IECDeviceType type) const
{
    switch (type)
    {
    case IECDeviceType::PrinterASCII:
        return { 4, 5 };

    case IECDeviceType::FoundryDrive:
    {
        QList<quint8> deviceNumbers;

        for (quint8 deviceNumber = 4; deviceNumber <= 30; ++deviceNumber)
            deviceNumbers.append(deviceNumber);

        return deviceNumbers;
    }
    }

    return {};
}
