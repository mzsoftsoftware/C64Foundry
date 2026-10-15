#pragma once

#include <QString>


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

struct C64KeyboardKeyLabel
{
    QString svgResource;
    C64KeyboardLabelPosition position = C64KeyboardLabelPosition::Center;
    C64KeyboardLabelModifier modifier = C64KeyboardLabelModifier::Always;
};
