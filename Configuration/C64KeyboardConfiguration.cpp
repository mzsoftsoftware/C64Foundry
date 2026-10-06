#include "C64KeyboardConfiguration.h"

#include "C64KeyboardDefaults.h"


QList<C64KeyboardMapping> C64KeyboardConfiguration::mappings(const HostPlatform::Type platform) const
{
    QList<C64KeyboardMapping> mappings = C64KeyboardDefaults::mappings(platform);

    // Overrides später hier anwenden.

    return mappings;
}