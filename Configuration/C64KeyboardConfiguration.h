#pragma once

#include <QList>
#include <QtGlobal>

#include "C64/Input/C64Keyboard.h"

enum class C64KeyboardMappingMode
{
    Momentary,
    Toggle
};
struct C64KeyboardMapping
{
    quint32 nativeScanCode;
    QList<C64Key> keys;
    C64KeyboardMappingMode mode = C64KeyboardMappingMode::Momentary;
};


class C64KeyboardConfiguration
{
public:
    C64KeyboardConfiguration() = default;

    QList<C64KeyboardMapping> mappings;
};
