#pragma once

#include "HostPlatform.h"
#include "C64KeyboardConfiguration.h"


class C64KeyboardDefaults
{
public:
    static C64KeyboardConfiguration configuration(HostPlatform::Type platform);
};
