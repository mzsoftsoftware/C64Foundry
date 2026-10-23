#pragma once

#include <QtGlobal>

namespace EmulatorIECDeviceEvent
{
enum class Type
{
    Added,
    Removed,
    ConfigurationChanged,
    Reset,
    Restored
};
}

Q_DECLARE_METATYPE(EmulatorIECDeviceEvent::Type)