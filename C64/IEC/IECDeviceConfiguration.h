#pragma once

#include <QtGlobal>


enum class IECDeviceType
{
    PrinterASCII
};


struct IECDeviceConfiguration
{
    quint64 id = 0;
    IECDeviceType type = IECDeviceType::PrinterASCII;
    quint8 deviceNumber = 4;
    bool enabled = false;

    bool operator==(const IECDeviceConfiguration& other) const
    {
        return id == other.id &&
               type == other.type &&
               deviceNumber == other.deviceNumber &&
               enabled == other.enabled;
    }
};
