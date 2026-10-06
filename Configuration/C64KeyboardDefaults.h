#pragma once

#include "HostPlatform.h"
#include "C64KeyboardConfiguration.h"


class C64KeyboardDefaults
{
public:
    static QList<C64KeyboardMapping> mappings(HostPlatform::Type platform);
};
