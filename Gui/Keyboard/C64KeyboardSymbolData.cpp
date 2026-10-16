#include "C64KeyboardSymbolData.h"


// Original C64 Character-ROM 901225-01, Uppercase/Graphics (erste 2048 Bytes).
// Index 0 = Shift, Index 1 = Commodore.
// Werte sind Bildschirmcodes, keine PETSCII-Codes.
static constexpr C64KeyboardSymbolBitmap keyBitmaps[12][2] =
    {
        // A: Shift $41, Commodore $70
        {
            {{ 0x08, 0x1C, 0x3E, 0x7F, 0x7F, 0x1C, 0x3E, 0x00 }},
            {{ 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x18, 0x18, 0x18 }}
        },
        // B: Shift $42, Commodore $7F
        {
            {{ 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 }},
            {{ 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F }}
        },
        // C: Shift $43, Commodore $7C
        {
            {{ 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00 }},
            {{ 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00 }}
        },
        // D: Shift $44, Commodore $6C
        {
            {{ 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 }},
            {{ 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F }}
        },
        // E: Shift $45, Commodore $71
        {
            {{ 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00 }},
            {{ 0x18, 0x18, 0x18, 0xFF, 0xFF, 0x00, 0x00, 0x00 }}
        },
        // F: Shift $46, Commodore $7B
        {
            {{ 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00 }},
            {{ 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0 }}
        },
        // G: Shift $47, Commodore $65
        {
            {{ 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 }},
            {{ 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0 }}
        },
        // H: Shift $48, Commodore $74
        {
            {{ 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C }},
            {{ 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0 }}
        },
        // I: Shift $49, Commodore $62
        {
            {{ 0x00, 0x00, 0x00, 0xE0, 0xF0, 0x38, 0x18, 0x18 }},
            {{ 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF }}
        },
        // J: Shift $4A, Commodore $75
        {
            {{ 0x18, 0x18, 0x1C, 0x0F, 0x07, 0x00, 0x00, 0x00 }},
            {{ 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0 }}
        },
        // K: Shift $4B, Commodore $61
        {
            {{ 0x18, 0x18, 0x38, 0xF0, 0xE0, 0x00, 0x00, 0x00 }},
            {{ 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0 }}
        },
        // L: Shift $4C, Commodore $76
        {
            {{ 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF }},
            {{ 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 }}
        }
};

// Erste Tastaturzeile: Shift- und Commodore-Grafikzeichen aus Character-ROM 901225-01.
static constexpr C64KeyboardSymbolBitmap bitmapPlusShift =
    {
        { 0x18, 0x18, 0x18, 0xFF, 0xFF, 0x18, 0x18, 0x18 }
};

static constexpr C64KeyboardSymbolBitmap bitmapPlusCommodore =
    {
        { 0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0x33, 0x33 }
};

static constexpr C64KeyboardSymbolBitmap bitmapMinusShift =
    {
        { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 }
};

static constexpr C64KeyboardSymbolBitmap bitmapMinusCommodore =
    {
        { 0xC0, 0xC0, 0x30, 0x30, 0xC0, 0xC0, 0x30, 0x30 }
};

static constexpr C64KeyboardSymbolBitmap bitmapPoundShift =
    {
        { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80 }
};

static constexpr C64KeyboardSymbolBitmap bitmapPoundCommodore =
    {
        { 0x00, 0x00, 0x00, 0x00, 0xCC, 0xCC, 0x33, 0x33 }
};


const C64KeyboardSymbolBitmap* C64KeyboardSymbolData::bitmap(const C64Key key, const C64KeyboardLabelModifier modifier)
{
    switch (key)
    {
    case C64Key::Plus:
        if (modifier == C64KeyboardLabelModifier::Shift) return &bitmapPlusShift;
        if (modifier == C64KeyboardLabelModifier::Commodore) return &bitmapPlusCommodore;
        return nullptr;

    case C64Key::Minus:
        if (modifier == C64KeyboardLabelModifier::Shift) return &bitmapMinusShift;
        if (modifier == C64KeyboardLabelModifier::Commodore) return &bitmapMinusCommodore;
        return nullptr;

    case C64Key::Pound:
        if (modifier == C64KeyboardLabelModifier::Shift) return &bitmapPoundShift;
        if (modifier == C64KeyboardLabelModifier::Commodore) return &bitmapPoundCommodore;
        return nullptr;

    default:
        break;
    }

    int keyIndex = -1;

    switch (key)
    {
    case C64Key::KeyA: keyIndex = 0; break;
    case C64Key::KeyB: keyIndex = 1; break;
    case C64Key::KeyC: keyIndex = 2; break;
    case C64Key::KeyD: keyIndex = 3; break;
    case C64Key::KeyE: keyIndex = 4; break;
    case C64Key::KeyF: keyIndex = 5; break;
    case C64Key::KeyG: keyIndex = 6; break;
    case C64Key::KeyH: keyIndex = 7; break;
    case C64Key::KeyI: keyIndex = 8; break;
    case C64Key::KeyJ: keyIndex = 9; break;
    case C64Key::KeyK: keyIndex = 10; break;
    case C64Key::KeyL: keyIndex = 11; break;
    default: return nullptr;
    }

    if (modifier == C64KeyboardLabelModifier::Shift)
        return &keyBitmaps[keyIndex][0];

    if (modifier == C64KeyboardLabelModifier::Commodore)
        return &keyBitmaps[keyIndex][1];

    return nullptr;
}
