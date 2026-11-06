#pragma once

#include <QList>
#include <QSharedPointer>
#include <QtGlobal>

#include "IECDeviceConfiguration.h"


struct IECSystemEvent
{
    enum class Type : quint8
    {
        Reset,
        DeviceAdded,
        DeviceRemoved,
        ConfigurationChanged,
        ConfigurationsRestored
    };

    Type type = Type::Reset;
    IECDeviceConfiguration::DeviceId deviceId = 0;
};


using IECSystemEventPtr = QSharedPointer<IECSystemEvent>;
using IECSystemEventList = QList<IECSystemEventPtr>;
