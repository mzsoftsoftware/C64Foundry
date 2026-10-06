#pragma once

#include <QString>

#include "HostPlatform.h"
#include "C64/C64ROMSet.h"
#include "C64KeyboardConfiguration.h"


class C64Configuration
{
public:
    C64Configuration() = default;

    QString name;
    HostPlatform platform;
    C64ROMSet romSet;
    C64KeyboardConfiguration keyboard;
};