#pragma once

#include <QString>

#include "C64KeyboardSymbol.h"


enum class C64KeyboardLabelPosition
{
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    Center,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

enum class C64KeyboardLabelModifier
{
    None,
    Shift,
    Commodore,
    Always
};

enum class C64KeyboardLabelType
{
    Text,
    Graphic,
    Symbol
};

struct C64KeyboardKeyLabel
{
    C64KeyboardLabelType type = C64KeyboardLabelType::Text;
    C64KeyboardSymbol symbol = C64KeyboardSymbol::ArrowUp;

    QString text;

    C64KeyboardLabelPosition position = C64KeyboardLabelPosition::Center;
    C64KeyboardLabelModifier modifier = C64KeyboardLabelModifier::Always;
};
