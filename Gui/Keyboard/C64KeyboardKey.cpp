#include "C64KeyboardKey.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>

#include <QtMath>


C64KeyboardKey::C64KeyboardKey(const C64Key key, const QString& label, const QRectF& rect)
    : m_key(key)
    , m_label(label)
    , m_rect(rect)
{
}

bool C64KeyboardKey::setHovered(const bool bHovered)
{
    if (m_bHovered == bHovered)
        return false;
    m_bHovered = bHovered;
    return true;
}
bool C64KeyboardKey::setMousePressed(const bool bPressed)
{
    if (m_bMousePressed == bPressed)
        return false;
    m_bMousePressed = bPressed;
    return true;
}
bool C64KeyboardKey::setKeyboardPressed(const bool bPressed)
{
    if (m_bKeyboardPressed == bPressed)
        return false;
    m_bKeyboardPressed = bPressed;
    return true;
}
bool C64KeyboardKey::setLatched(const bool bLatched)
{
    if (m_bLatched == bLatched)
        return false;
    m_bLatched = bLatched;
    return true;
}
bool C64KeyboardKey::setModifiers(const C64KeyboardModifiers& modifiers)
{
    if (m_modifiers == modifiers)
        return false;
    m_modifiers = modifiers;
    return true;
}

void C64KeyboardKey::paint(QPainter* ptrPainter) const
{
    constexpr qreal KeyMargin = 0.04;
    constexpr qreal KeyRadius = 0.08;
    constexpr qreal TextMargin = 0.12;

    const QRectF keyRect = m_rect.adjusted(KeyMargin, KeyMargin, -KeyMargin, -KeyMargin);
    const QRectF textRect = keyRect.adjusted(TextMargin, TextMargin, -TextMargin, -TextMargin);

    ptrPainter->save();

    ptrPainter->setPen(QPen(QColor(60, 55, 50), 0.02));

    if (isPressed())
        ptrPainter->setBrush(QColor(150, 140, 120));
    else if (isHovered())
        ptrPainter->setBrush(QColor(225, 215, 190));
    else
        ptrPainter->setBrush(QColor(200, 190, 170));

    ptrPainter->drawRoundedRect(keyRect, KeyRadius, KeyRadius);

    // Text in einem eigenen lokalen Koordinatensystem zeichnen.
    // Die Schrift wird proportional an die verfügbare Fläche angepasst.

    QFont font = ptrPainter->font();
    font.setPixelSize(100);
    ptrPainter->setFont(font);

    const QFontMetricsF metrics(font);
    const qreal textWidth = metrics.horizontalAdvance(m_label);
    const qreal textHeight = metrics.height();

    if (textWidth > 0.0 && textHeight > 0.0 && !textRect.isEmpty())
    {
        const qreal textScale = qMin(textRect.width() / textWidth, textRect.height() / textHeight);

        ptrPainter->setPen(QColor(30, 30, 30));
        ptrPainter->translate(textRect.center());
        ptrPainter->scale(textScale, textScale);

        const QRectF localTextRect(-textWidth / 2.0, -textHeight / 2.0, textWidth, textHeight);

        ptrPainter->drawText(localTextRect, Qt::AlignCenter, m_label);
    }

    ptrPainter->restore();
}
