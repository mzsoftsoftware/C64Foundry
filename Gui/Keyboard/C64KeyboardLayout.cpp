#include "C64KeyboardLayout.h"


C64KeyboardLayout::C64KeyboardLayout()
{
    // Row 1
    C64KeyboardKey* ptrKeyArrowLeft = addKey(C64Key::ArrowLeft, "←", 0.25, 0.0);
    ptrKeyArrowLeft->addLabel(QStringLiteral("←"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKey1 = addKey(C64Key::Key1, "1", 1.25, 0.0);
    ptrKey1->addLabel(QStringLiteral("!"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey1->addLabel(QStringLiteral("1"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey1->addLabel(QStringLiteral("BLK"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey2 = addKey(C64Key::Key2, "2", 2.25, 0.0);
    ptrKey2->addLabel(QStringLiteral("\""), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey2->addLabel(QStringLiteral("2"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey2->addLabel(QStringLiteral("WHT"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey3 = addKey(C64Key::Key3, "3", 3.25, 0.0);
    ptrKey3->addLabel(QStringLiteral("#"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey3->addLabel(QStringLiteral("3"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey3->addLabel(QStringLiteral("RED"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey4 = addKey(C64Key::Key4, "4", 4.25, 0.0);
    ptrKey4->addLabel(QStringLiteral("$"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey4->addLabel(QStringLiteral("4"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey4->addLabel(QStringLiteral("CYN"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey5 = addKey(C64Key::Key5, "5", 5.25, 0.0);
    ptrKey5->addLabel(QStringLiteral("%"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey5->addLabel(QStringLiteral("5"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey5->addLabel(QStringLiteral("PUR"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey6 = addKey(C64Key::Key6, "6", 6.25, 0.0);
    ptrKey6->addLabel(QStringLiteral("&"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey6->addLabel(QStringLiteral("6"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey6->addLabel(QStringLiteral("GRN"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey7 = addKey(C64Key::Key7, "7", 7.25, 0.0);
    ptrKey7->addLabel(QStringLiteral("'"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey7->addLabel(QStringLiteral("7"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey7->addLabel(QStringLiteral("BLU"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey8 = addKey(C64Key::Key8, "8", 8.25, 0.0);
    ptrKey8->addLabel(QStringLiteral("("), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey8->addLabel(QStringLiteral("8"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey8->addLabel(QStringLiteral("YEL"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey9 = addKey(C64Key::Key9, "9", 9.25, 0.0);
    ptrKey9->addLabel(QStringLiteral(")"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey9->addLabel(QStringLiteral("9"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey9->addLabel(QStringLiteral("RVS\nON"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKey0 = addKey(C64Key::Key0, "0", 10.25, 0.0);
    ptrKey0->addLabel(QStringLiteral(" "), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);
    ptrKey0->addLabel(QStringLiteral("0"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKey0->addLabel(QStringLiteral("RVS\nOFF"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKeyPlus = addKey(C64Key::Plus, "+", 11.25, 0.0);
    ptrKeyPlus->addLabel(QStringLiteral("+"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyMinus = addKey(C64Key::Minus, "-", 12.25, 0.0);
    ptrKeyMinus->addLabel(QStringLiteral("-"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyPound = addKey(C64Key::Pound, "£", 13.25, 0.0);
    ptrKeyPound->addLabel(QStringLiteral("£"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyHomeClear = addKey(C64Key::HomeClear, "HOME", 14.25, 0.0);
    ptrKeyHomeClear->addLabel(QStringLiteral("CLR"), C64KeyboardLabelPosition::TopCenter, C64KeyboardLabelModifier::Shift);
    ptrKeyHomeClear->addLabel(QStringLiteral("HOME"), C64KeyboardLabelPosition::BottomCenter, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyInsertDelete = addKey(C64Key::InsertDelete, "DEL", 15.25, 0.0);
    ptrKeyInsertDelete->addLabel(QStringLiteral("INST"), C64KeyboardLabelPosition::TopCenter, C64KeyboardLabelModifier::Shift);
    ptrKeyInsertDelete->addLabel(QStringLiteral("DEL"), C64KeyboardLabelPosition::BottomCenter, C64KeyboardLabelModifier::None);

    // Row 2
    C64KeyboardKey* ptrKeyControl = addKey(C64Key::Control, "CTRL", 0.25, 1.0, 1.5);
    ptrKeyControl->addLabel(QStringLiteral("CTRL"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKeyQ = addKey(C64Key::KeyQ, "Q", 1.75, 1.0);
    ptrKeyQ->addLabel(QStringLiteral("Q"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyW = addKey(C64Key::KeyW, "W", 2.75, 1.0);
    ptrKeyW->addLabel(QStringLiteral("W"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyE = addKey(C64Key::KeyE, "E", 3.75, 1.0);
    ptrKeyE->addLabel(QStringLiteral("E"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyR = addKey(C64Key::KeyR, "R", 4.75, 1.0);
    ptrKeyR->addLabel(QStringLiteral("R"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyT = addKey(C64Key::KeyT, "T", 5.75, 1.0);
    ptrKeyT->addLabel(QStringLiteral("T"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyY = addKey(C64Key::KeyY, "Y", 6.75, 1.0);
    ptrKeyY->addLabel(QStringLiteral("Y"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyU = addKey(C64Key::KeyU, "U", 7.75, 1.0);
    ptrKeyU->addLabel(QStringLiteral("U"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyI = addKey(C64Key::KeyI, "I", 8.75, 1.0);
    ptrKeyI->addLabel(QStringLiteral("I"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyO = addKey(C64Key::KeyO, "O", 9.75, 1.0);
    ptrKeyO->addLabel(QStringLiteral("O"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyP = addKey(C64Key::KeyP, "P", 10.75, 1.0);
    ptrKeyP->addLabel(QStringLiteral("P"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyAt = addKey(C64Key::At, "@", 11.75, 1.0);
    ptrKeyAt->addLabel(QStringLiteral("@"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyAsterisk = addKey(C64Key::Asterisk, "*", 12.75, 1.0);
    ptrKeyAsterisk->addLabel(QStringLiteral("*"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyArrowUp = addKey(C64Key::ArrowUp, "↑", 13.75, 1.0);
    ptrKeyArrowUp->addLabel(QStringLiteral("↑"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyArrowUp->addLabel(QStringLiteral("π"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyRestore = addKey(C64Key::Restore, "RESTORE", 14.75, 1.0, 1.5);
    ptrKeyRestore->addLabel(QStringLiteral("RESTORE"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    // Row 3
    C64KeyboardKey* ptrKeyRunStop = addKey(C64Key::RunStop, "RUN", 0.0, 2.0);
    ptrKeyRunStop->addLabel(QStringLiteral("RUN\nSTOP"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKeyShiftLock = addKey(C64Key::ShiftLock, "LOCK", 1.0, 2.0);
    ptrKeyShiftLock->addLabel(QStringLiteral("SHIFT\nLOCK"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKeyA = addKey(C64Key::KeyA, "A", 2.0, 2.0);
    ptrKeyA->addLabel(QStringLiteral("A"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyA->addGraphicLabel(C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Shift);
    ptrKeyA->addGraphicLabel(C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::Commodore);

    C64KeyboardKey* ptrKeyS = addKey(C64Key::KeyS, "S", 3.0, 2.0);
    ptrKeyS->addLabel(QStringLiteral("S"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyD = addKey(C64Key::KeyD, "D", 4.0, 2.0);
    ptrKeyD->addLabel(QStringLiteral("D"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyF = addKey(C64Key::KeyF, "F", 5.0, 2.0);
    ptrKeyF->addLabel(QStringLiteral("F"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyG = addKey(C64Key::KeyG, "G", 6.0, 2.0);
    ptrKeyG->addLabel(QStringLiteral("G"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyH = addKey(C64Key::KeyH, "H", 7.0, 2.0);
    ptrKeyH->addLabel(QStringLiteral("H"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyJ = addKey(C64Key::KeyJ, "J", 8.0, 2.0);
    ptrKeyJ->addLabel(QStringLiteral("J"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyK = addKey(C64Key::KeyK, "K", 9.0, 2.0);
    ptrKeyK->addLabel(QStringLiteral("K"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyL = addKey(C64Key::KeyL, "L", 10.0, 2.0);
    ptrKeyL->addLabel(QStringLiteral("L"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyColon = addKey(C64Key::Colon, ":", 11.0, 2.0);
    ptrKeyColon->addLabel(QStringLiteral(":"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyColon->addLabel(QStringLiteral("["), C64KeyboardLabelPosition::TopRight, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeySemicolon = addKey(C64Key::Semicolon, ";", 12.0, 2.0);
    ptrKeySemicolon->addLabel(QStringLiteral(";"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeySemicolon->addLabel(QStringLiteral("]"), C64KeyboardLabelPosition::TopRight, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyEquals = addKey(C64Key::Equals, "=", 13.0, 2.0);
    ptrKeyEquals->addLabel(QStringLiteral("="), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyReturn = addKey(C64Key::Return, "RETURN", 14.0, 2.0, 2.0);
    ptrKeyReturn->addLabel(QStringLiteral("RETURN"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    // Row 4
    C64KeyboardKey* ptrKeyCommodore = addKey(C64Key::Commodore, "CBM", 0.0, 3.0);
    ptrKeyCommodore->addLabel(QStringLiteral("CBM"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKeyLeftShift = addKey(C64Key::LeftShift, "SHIFT", 1.0, 3.0, 1.5);
    ptrKeyLeftShift->addLabel(QStringLiteral("SHIFT"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKeyZ = addKey(C64Key::KeyZ, "Z", 2.5, 3.0);
    ptrKeyZ->addLabel(QStringLiteral("Z"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyX = addKey(C64Key::KeyX, "X", 3.5, 3.0);
    ptrKeyX->addLabel(QStringLiteral("X"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyC = addKey(C64Key::KeyC, "C", 4.5, 3.0);
    ptrKeyC->addLabel(QStringLiteral("C"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyV = addKey(C64Key::KeyV, "V", 5.5, 3.0);
    ptrKeyV->addLabel(QStringLiteral("V"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyB = addKey(C64Key::KeyB, "B", 6.5, 3.0);
    ptrKeyB->addLabel(QStringLiteral("B"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyN = addKey(C64Key::KeyN, "N", 7.5, 3.0);
    ptrKeyN->addLabel(QStringLiteral("N"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyM = addKey(C64Key::KeyM, "M", 8.5, 3.0);
    ptrKeyM->addLabel(QStringLiteral("M"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyComma = addKey(C64Key::Comma, ",", 9.5, 3.0);
    ptrKeyComma->addLabel(QStringLiteral(","), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKeyComma->addLabel(QStringLiteral("<"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyPeriod = addKey(C64Key::Period, ".", 10.5, 3.0);
    ptrKeyPeriod->addLabel(QStringLiteral("."), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKeyPeriod->addLabel(QStringLiteral(">"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeySlash = addKey(C64Key::Slash, "/", 11.5, 3.0);
    ptrKeySlash->addLabel(QStringLiteral("/"), C64KeyboardLabelPosition::BottomLeft, C64KeyboardLabelModifier::None);
    ptrKeySlash->addLabel(QStringLiteral("?"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyRightShift = addKey(C64Key::RightShift, "SHIFT", 12.5, 3.0, 1.5);
    ptrKeyRightShift->addLabel(QStringLiteral("SHIFT"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    C64KeyboardKey* ptrKeyCursorUpDown = addKey(C64Key::CursorUpDown, "CRSR", 14.0, 3.0);
    ptrKeyCursorUpDown->addSymbolLabel(C64KeyboardSymbol::DoubleArrowUp, C64KeyboardLabelPosition::TopCenter, C64KeyboardLabelModifier::Shift);
    ptrKeyCursorUpDown->addLabel(QStringLiteral("CRSR"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);
    ptrKeyCursorUpDown->addSymbolLabel(C64KeyboardSymbol::DoubleArrowDown, C64KeyboardLabelPosition::BottomCenter, C64KeyboardLabelModifier::None);

    C64KeyboardKey* ptrKeyCursorLeftRight = addKey(C64Key::CursorLeftRight, "CRSR", 15.0, 3.0);
    ptrKeyCursorLeftRight->addSymbolLabel(C64KeyboardSymbol::DoubleArrowLeft, C64KeyboardLabelPosition::TopCenter, C64KeyboardLabelModifier::Shift);
    ptrKeyCursorLeftRight->addLabel(QStringLiteral("CRSR"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);
    ptrKeyCursorLeftRight->addSymbolLabel(C64KeyboardSymbol::DoubleArrowRight, C64KeyboardLabelPosition::BottomCenter, C64KeyboardLabelModifier::None);

    // Space bar
    C64KeyboardKey* ptrKeySpace = addKey(C64Key::Space, "SPACE", 2.75, 4.0, 9.0);
    ptrKeySpace->addLabel(QStringLiteral("SPACE"), C64KeyboardLabelPosition::Center, C64KeyboardLabelModifier::Always);

    // Function keys
    C64KeyboardKey* ptrKeyF1F2 = addKey(C64Key::F1F2, "F1/F2", 16.75, 0.0, 1.5);
    ptrKeyF1F2->addLabel(QStringLiteral("F1"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyF1F2->addLabel(QStringLiteral("F2"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyF3F4 = addKey(C64Key::F3F4, "F3/F4", 16.75, 1.0, 1.5);
    ptrKeyF3F4->addLabel(QStringLiteral("F3"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyF3F4->addLabel(QStringLiteral("F4"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyF5F6 = addKey(C64Key::F5F6, "F5/F6", 16.75, 2.0, 1.5);
    ptrKeyF5F6->addLabel(QStringLiteral("F5"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyF5F6->addLabel(QStringLiteral("F6"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Shift);

    C64KeyboardKey* ptrKeyF7F8 = addKey(C64Key::F7F8, "F7/F8", 16.75, 3.0, 1.5);
    ptrKeyF7F8->addLabel(QStringLiteral("F7"), C64KeyboardLabelPosition::TopLeft, C64KeyboardLabelModifier::None);
    ptrKeyF7F8->addLabel(QStringLiteral("F8"), C64KeyboardLabelPosition::BottomRight, C64KeyboardLabelModifier::Shift);

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
