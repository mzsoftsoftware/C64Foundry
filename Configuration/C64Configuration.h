#pragma once

#include <QString>
#include <QList>
#include <QSharedPointer>

#include "HostPlatform.h"
#include "C64/C64ROMSet.h"
#include "C64KeyboardConfiguration.h"
#include "C64/IEC/IECDeviceConfiguration.h"


class C64Configuration
{
public:
    C64Configuration() = default;

    QString name;
    HostPlatform platform;
    C64ROMSet romSet;
    C64KeyboardConfiguration keyboard;

    QList<QSharedPointer<IECDeviceConfiguration>> iecDevices;
};
