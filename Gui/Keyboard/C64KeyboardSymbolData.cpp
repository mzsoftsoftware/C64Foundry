#include "C64KeyboardSymbolData.h"


static constexpr C64KeyboardSymbolBitmap bitmapKeyAShift =
    {
        {
            0b00011000,
            0b00111100,
            0b01111110,
            0b11111111,
            0b00011000,
            0b00011000,
            0b00011000,
            0b00011000
        }
};


static constexpr C64KeyboardSymbolBitmap bitmapKeyACommodore =
    {
        {
            0b11110000,
            0b11110000,
            0b11110000,
            0b11110000,
            0b00001111,
            0b00001111,
            0b00001111,
            0b00001111
        }
};


const C64KeyboardSymbolBitmap* C64KeyboardSymbolData::bitmap(const C64Key key, const C64KeyboardLabelModifier modifier)
{
    switch (key)
    {
    case C64Key::KeyA:
    {
        switch (modifier)
        {
        case C64KeyboardLabelModifier::Shift:
            return &bitmapKeyAShift;

        case C64KeyboardLabelModifier::Commodore:
            return &bitmapKeyACommodore;

        default:
            return nullptr;
        }
    }

    default:
        return nullptr;
    }
}