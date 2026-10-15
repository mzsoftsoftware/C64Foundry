#pragma once

#include <QtGlobal>

#include "C64/Input/C64Keyboard.h"
#include "Gui/Keyboard/C64KeyboardKeyLabel.h"


struct C64KeyboardSymbolBitmap
{
    quint8 rows[8];
};


class C64KeyboardSymbolData
{
public:
    static const C64KeyboardSymbolBitmap* bitmap(C64Key key, C64KeyboardLabelModifier modifier);
};
