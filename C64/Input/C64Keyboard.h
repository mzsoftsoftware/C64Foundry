#pragma once

#include <QtGlobal>

enum class C64Key
{
    //
    // Keyboard matrix
    //
    InsertDelete,
    Return,
    CursorLeftRight,
    F7F8,
    F1F2,
    F3F4,
    F5F6,
    CursorUpDown,

    Key3,
    KeyW,
    KeyA,
    Key4,
    KeyZ,
    KeyS,
    KeyE,
    LeftShift,

    Key5,
    KeyR,
    KeyD,
    Key6,
    KeyC,
    KeyF,
    KeyT,
    KeyX,

    Key7,
    KeyY,
    KeyG,
    Key8,
    KeyB,
    KeyH,
    KeyU,
    KeyV,

    Key9,
    KeyI,
    KeyJ,
    Key0,
    KeyM,
    KeyK,
    KeyO,
    KeyN,

    Plus,
    KeyP,
    KeyL,
    Minus,
    Period,
    Colon,
    At,
    Comma,

    Pound,
    Asterisk,
    Semicolon,
    HomeClear,
    RightShift,
    Equals,
    ArrowUp,
    Slash,

    Key1,
    ArrowLeft,
    Control,
    Key2,
    Space,
    Commodore,
    KeyQ,
    RunStop,

    //
    // Special keys outside the keyboard matrix
    //
    ShiftLock,
    Restore
};


class C64Keyboard
{
public:
    C64Keyboard();

    void press(C64Key key);
    void release(C64Key key);

    quint8 portAInputs(quint8 portBPins) const;
    quint8 portBInputs(quint8 portAPins) const;

private:
    quint8 m_matrixToPortA[8];
    quint8 m_matrixToPortB[8];

    bool m_leftShiftPressed = false;
    bool m_shiftLockPressed = false;
};
