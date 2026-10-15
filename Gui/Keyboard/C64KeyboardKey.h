#pragma once

#include <QRectF>
#include <QString>
#include <QVector>

#include "C64/Input/C64Keyboard.h"
#include "C64KeyboardModifiers.h"
#include "C64KeyboardKeyLabel.h"

class QPainter;


class C64KeyboardKey
{
public:
    C64KeyboardKey(C64Key key, const QString& label, const QRectF& rect);

    // Getter
    C64Key key() const                                              { return m_key; }
    QRectF rect() const                                             { return m_rect; }
    const QVector<C64KeyboardKeyLabel>& labels() const              { return m_labels; }

    bool isHovered() const                                          { return m_bHovered; }
    bool isPressed() const                                          { return m_bMousePressed || m_bKeyboardPressed || m_bLatched; }
    bool isLatched() const                                          { return m_bLatched; }

    // Setter
    bool setHovered(bool bHovered);
    bool setMousePressed(bool bPressed);
    bool setKeyboardPressed(bool bPressed);
    bool setLatched(bool bLatched);
    bool setModifiers(const C64KeyboardModifiers& modifiers);

    bool contains(const QPointF& position) const                    { return m_rect.contains(position); }
    bool addLabel(const QString& text, C64KeyboardLabelPosition position, C64KeyboardLabelModifier modifier);
    bool addGraphicLabel(C64KeyboardLabelPosition position, C64KeyboardLabelModifier modifier);

    void paint(QPainter* ptrPainter) const;

private:
    bool appendLabel(const C64KeyboardKeyLabel& label);

private:
    C64Key m_key;
    QString m_label;
    QRectF m_rect;
    QVector<C64KeyboardKeyLabel> m_labels;

    C64KeyboardModifiers m_modifiers;

    bool m_bHovered = false;
    bool m_bMousePressed = false;
    bool m_bKeyboardPressed = false;
    bool m_bLatched = false;
};
