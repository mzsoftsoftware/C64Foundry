#pragma once

#include <QtGlobal>

enum class MOS6510StatusFlag : quint8
{
    Carry               = 0x01,
    Zero                = 0x02,
    InterruptDisable    = 0x04,
    Decimal             = 0x08,
    Break               = 0x10,
    Unused              = 0x20,
    Overflow            = 0x40,
    Negative            = 0x80
};
