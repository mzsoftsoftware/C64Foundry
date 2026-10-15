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

enum class C64KeyboardLabelType
{
    Text,
    Graphic
};

struct C64KeyboardKeyLabel
{
    C64KeyboardLabelType type = C64KeyboardLabelType::Text;

    QString text;

    C64KeyboardLabelPosition position = C64KeyboardLabelPosition::Center;
    C64KeyboardLabelModifier modifier = C64KeyboardLabelModifier::Always;
};
