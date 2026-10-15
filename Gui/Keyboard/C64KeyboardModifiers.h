#pragma once


struct C64KeyboardModifiers
{
    bool leftShift = false;
    bool rightShift = false;
    bool commodore = false;
    bool control = false;
    bool shiftLock = false;

    bool isShiftActive() const
    {
        return leftShift || rightShift || shiftLock;
    }

    bool operator==(const C64KeyboardModifiers& other) const = default;
};
