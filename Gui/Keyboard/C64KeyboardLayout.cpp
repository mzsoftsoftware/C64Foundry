#include "C64KeyboardLayout.h"


C64KeyboardLayout::C64KeyboardLayout()
{
    // Row 1
    addKey(C64Key::ArrowLeft,       "←",       0.25, 0.0);
    addKey(C64Key::Key1,            "1",       1.25, 0.0);
    addKey(C64Key::Key2,            "2",       2.25, 0.0);
    addKey(C64Key::Key3,            "3",       3.25, 0.0);
    addKey(C64Key::Key4,            "4",       4.25, 0.0);
    addKey(C64Key::Key5,            "5",       5.25, 0.0);
    addKey(C64Key::Key6,            "6",       6.25, 0.0);
    addKey(C64Key::Key7,            "7",       7.25, 0.0);
    addKey(C64Key::Key8,            "8",       8.25, 0.0);
    addKey(C64Key::Key9,            "9",       9.25, 0.0);
    addKey(C64Key::Key0,            "0",      10.25, 0.0);
    addKey(C64Key::Plus,            "+",      11.25, 0.0);
    addKey(C64Key::Minus,           "-",      12.25, 0.0);
    addKey(C64Key::Pound,           "£",      13.25, 0.0);
    addKey(C64Key::HomeClear,       "HOME",   14.25, 0.0);
    addKey(C64Key::InsertDelete,    "DEL",    15.25, 0.0);

    // Row 2
    addKey(C64Key::Control,         "CTRL",    0.25, 1.0, 1.5);
    addKey(C64Key::KeyQ,            "Q",       1.75, 1.0);
    addKey(C64Key::KeyW,            "W",       2.75, 1.0);
    addKey(C64Key::KeyE,            "E",       3.75, 1.0);
    addKey(C64Key::KeyR,            "R",       4.75, 1.0);
    addKey(C64Key::KeyT,            "T",       5.75, 1.0);
    addKey(C64Key::KeyY,            "Y",       6.75, 1.0);
    addKey(C64Key::KeyU,            "U",       7.75, 1.0);
    addKey(C64Key::KeyI,            "I",       8.75, 1.0);
    addKey(C64Key::KeyO,            "O",       9.75, 1.0);
    addKey(C64Key::KeyP,            "P",      10.75, 1.0);
    addKey(C64Key::At,              "@",      11.75, 1.0);
    addKey(C64Key::Asterisk,        "*",      12.75, 1.0);
    addKey(C64Key::ArrowUp,         "↑",      13.75, 1.0);
    addKey(C64Key::Restore,         "RESTORE",14.75, 1.0, 1.5);

    // Row 3
    addKey(C64Key::RunStop,         "RUN",     0.0, 2.0);
    addKey(C64Key::ShiftLock,       "LOCK",    1.0, 2.0);

    C64KeyboardKey* ptrKeyA = addKey(C64Key::KeyA,            "A",       2.0, 2.0);
    ptrKeyA->addLabel(QStringLiteral("A"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyA->addGraphicLabel(C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Shift);
    ptrKeyA->addGraphicLabel(C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::Commodore);

    addKey(C64Key::KeyS,            "S",       3.0, 2.0);
    addKey(C64Key::KeyD,            "D",       4.0, 2.0);
    addKey(C64Key::KeyF,            "F",       5.0, 2.0);
    addKey(C64Key::KeyG,            "G",       6.0, 2.0);
    addKey(C64Key::KeyH,            "H",       7.0, 2.0);
    addKey(C64Key::KeyJ,            "J",       8.0, 2.0);
    addKey(C64Key::KeyK,            "K",       9.0, 2.0);
    addKey(C64Key::KeyL,            "L",      10.0, 2.0);
    addKey(C64Key::Colon,           ":",      11.0, 2.0);
    addKey(C64Key::Semicolon,       ";",      12.0, 2.0);
    addKey(C64Key::Equals,          "=",      13.0, 2.0);
    addKey(C64Key::Return,          "RETURN", 14.0, 2.0, 2.0);

    // Row 4
    addKey(C64Key::Commodore,       "CBM",     0.0, 3.0);
    addKey(C64Key::LeftShift,       "SHIFT",   1.0, 3.0, 1.5);
    addKey(C64Key::KeyZ,            "Z",       2.5, 3.0);
    addKey(C64Key::KeyX,            "X",       3.5, 3.0);
    addKey(C64Key::KeyC,            "C",       4.5, 3.0);
    addKey(C64Key::KeyV,            "V",       5.5, 3.0);
    addKey(C64Key::KeyB,            "B",       6.5, 3.0);
    addKey(C64Key::KeyN,            "N",       7.5, 3.0);
    addKey(C64Key::KeyM,            "M",       8.5, 3.0);
    addKey(C64Key::Comma,           ",",       9.5, 3.0);
    addKey(C64Key::Period,          ".",      10.5, 3.0);
    addKey(C64Key::Slash,           "/",      11.5, 3.0);
    addKey(C64Key::RightShift,      "SHIFT",  12.5, 3.0, 1.5);

    C64KeyboardKey* ptrKeyCursor = addKey(C64Key::CursorUpDown, "CRSR", 14.0, 3.0);
    ptrKeyCursor->addSymbolLabel(C64KeyboardSymbol::DoubleArrowUp, C64KeyboardLabelPosition::TopCenter, C64KeyboardLabelModifier::Shift);
    ptrKeyCursor->addLabel(QStringLiteral("CRSR"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);
    ptrKeyCursor->addSymbolLabel(C64KeyboardSymbol::DoubleArrowDown, C64KeyboardLabelPosition::BottomCenter, C64KeyboardLabelModifier::None);

    addKey(C64Key::CursorLeftRight, "CRSR",   15.0, 3.0);

    // Space bar
    C64KeyboardKey* ptrKeySpace = addKey(C64Key::Space, "SPACE", 2.75, 4.0, 9.0);
    ptrKeySpace->addLabel(QStringLiteral("SPACE"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    // Function keys
    addKey(C64Key::F1F2,            "F1/F2",  16.75, 0.0, 1.5);
    addKey(C64Key::F3F4,            "F3/F4",  16.75, 1.0, 1.5);
    addKey(C64Key::F5F6,            "F5/F6",  16.75, 2.0, 1.5);
    addKey(C64Key::F7F8,            "F7/F8",  16.75, 3.0, 1.5);

    updateBounds();
}

C64KeyboardKey* C64KeyboardLayout::keyAt(const QPointF& position)
{
    for (C64KeyboardKey& key : m_keys)
    {
        if (key.contains(position))
            return &key;
    }
    return nullptr;
}
C64KeyboardKey* C64KeyboardLayout::keyById(const C64Key keyId)
{
    for (C64KeyboardKey& key : m_keys)
    {
        if (key.key() == keyId)
            return &key;
    }
    return nullptr;
}

C64KeyboardKey* C64KeyboardLayout::addKey(const C64Key key, const QString& label, const qreal x, const qreal y, const qreal width)
{    
    m_keys.append(C64KeyboardKey(key, label, QRectF(x, y, width, 1.0)));
    return &m_keys.last();
}

void C64KeyboardLayout::updateBounds()
{
    m_bounds = QRectF();

    for (const C64KeyboardKey& key : m_keys)
        m_bounds = m_bounds.united(key.rect());
}

const C64KeyboardKey* C64KeyboardLayout::keyAt(const QPointF& position) const
{
    for (const C64KeyboardKey& key : m_keys)
    {
        if (key.rect().contains(position))
            return &key;
    }

    return nullptr;
}
