#pragma once

#include "C64/Input/C64Keyboard.h"


enum class InputEventType
{
    KeyPress,
    KeyRelease
};


struct InputEvent
{
    InputEventType type;
    C64Key key;
};
