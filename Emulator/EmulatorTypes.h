#pragma once

#include <QObject>

namespace Emulator
{
Q_NAMESPACE

enum class State
{
    PoweredOff,
    Running,
    Paused
};
Q_ENUM_NS(State)

enum class Speed
{
    Half,
    Normal,
    Double,
    Five,
    Warp
};
Q_ENUM_NS(Speed)
}