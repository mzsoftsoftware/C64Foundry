#pragma once

#include <QtGlobal>

namespace EmulatorIECDeviceEvent
{
enum class Type
{
    Added,
    Removed,
    ConfigurationChanged,
    Reset
};
}

Q_DECLARE_METATYPE(EmulatorIECDeviceEvent::Type)