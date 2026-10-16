#include "C64KeyboardKey.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QtMath>
#include <QDebug>

#include "C64KeyboardSymbolData.h"
#include "C64KeyboardSymbolPainter.h"


C64KeyboardKey::C64KeyboardKey(const C64Key key, const QString& label, const QRectF& rect)
    : m_key(key)
    , m_label(label)
    , m_rect(rect)
{
}

bool C64KeyboardKey::addLabel(const QString& text, const C64KeyboardLabelPosition position, const C64KeyboardLabelModifier modifier)
{
    if (text.isEmpty())
    {
        qWarning() << "C64KeyboardKey: Empty text for key:" << static_cast<int>(m_key);
        return false;
    }
    C64KeyboardKeyLabel label;
    label.type = C64KeyboardLabelType::Text;
    label.text = text;
    label.position = position;
    label.modifier = modifier;
    return appendLabel(label);
}
bool C64KeyboardKey::addGraphicLabel(const C64KeyboardLabelPosition position, const C64KeyboardLabelModifier modifier)
{
    if (modifier != C64KeyboardLabelModifier::Shift && modifier != C64KeyboardLabelModifier::Commodore)
    {
        qWarning() << "C64KeyboardKey: Invalid graphic modifier for key:" << static_cast<int>(m_key);
        return false;
    }
    if (C64KeyboardSymbolData::bitmap(m_key, modifier) == nullptr)
    {
        qWarning() << "C64KeyboardKey: No graphic bitmap defined for key:" << static_cast<int>(m_key) << "modifier:" << static_cast<int>(modifier);
        return false;
    }
    C64KeyboardKeyLabel label;
    label.type = C64KeyboardLabelType::Graphic;
    label.position = position;
    label.modifier = modifier;
    return appendLabel(label);
}
bool C64KeyboardKey::addSymbolLabel(const C64KeyboardSymbol symbol, const C64KeyboardLabelPosition position, const C64KeyboardLabelModifier modifier)
{
    C64KeyboardKeyLabel label;
    label.type = C64KeyboardLabelType::Symbol;
    label.symbol = symbol;
    label.position = position;
    label.modifier = modifier;

    return appendLabel(label);
}

bool C64KeyboardKey::appendLabel(const C64KeyboardKeyLabel& label)
{
    for (const C64KeyboardKeyLabel& existingLabel : m_labels)
    {
        if (existingLabel.position == label.position)
        {
            qWarning() << "C64KeyboardKey: Duplicate label position for key:" << static_cast<int>(m_key);
            return false;
        }
    }

    m_labels.append(label);
    return true;
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
    constexpr qreal TextMargin = 0.08;

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

    if (m_labels.isEmpty())
    {
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

            ptrPainter->drawText(QRectF(-textWidth / 2.0, -textHeight / 2.0, textWidth, textHeight), Qt::AlignCenter, m_label);
        }
    }
    else
    {
        paintLabels(ptrPainter, textRect);
    }

    ptrPainter->restore();
}

QRectF C64KeyboardKey::calculateLabelRect(const C64KeyboardLabelPosition position, const QRectF& rect) const
{
    constexpr qreal MinimumCenterSize = 0.04;
    constexpr qreal CenterTextHeight = 0.24;

    if (rect.isEmpty())
        return QRectF();

    bool usedColumns[3] = { false, false, false };
    bool usedRows[3] = { false, false, false };

    bool centerHasLabel = false;

    for (const C64KeyboardKeyLabel& label : m_labels)
    {
        const int index = static_cast<int>(label.position);

        usedColumns[index % 3] = true;
        usedRows[index / 3] = true;

        if (label.position == C64KeyboardLabelPosition::Center)
            centerHasLabel = true;
    }

    const int outerColumns = static_cast<int>(usedColumns[0]) + static_cast<int>(usedColumns[2]);
    const int outerRows = static_cast<int>(usedRows[0]) + static_cast<int>(usedRows[2]);

    qreal centerWidth = qMin(MinimumCenterSize, rect.width());
    qreal centerHeight = qMin(MinimumCenterSize, rect.height());

    if (centerHasLabel)
    {
        for (const C64KeyboardKeyLabel& label : m_labels)
        {
            if (label.position != C64KeyboardLabelPosition::Center)
                continue;

            if (label.type == C64KeyboardLabelType::Text)
            {
                QFont font;
                font.setBold(true);
                font.setPixelSize(100);

                const QFontMetricsF metrics(font);

                const qreal textWidth = metrics.horizontalAdvance(label.text);
                const qreal textHeight = metrics.height();

                if (textWidth > 0.0 && textHeight > 0.0)
                {
                    const qreal desiredHeight = qMin(CenterTextHeight, rect.height());
                    const qreal desiredWidth = desiredHeight * textWidth / textHeight;

                    centerWidth = qMax(centerWidth, qMin(desiredWidth, rect.width()));
                    centerHeight = qMax(centerHeight, desiredHeight);
                }
            }
            else
            {
                const qreal desiredSize = qMin(CenterTextHeight, qMin(rect.width(), rect.height()));

                centerWidth = qMax(centerWidth, desiredSize);
                centerHeight = qMax(centerHeight, desiredSize);
            }

            break;
        }
    }

    qreal widths[3] = { 0.0, centerWidth, 0.0 };
    qreal heights[3] = { 0.0, centerHeight, 0.0 };

    if (outerColumns > 0)
    {
        const qreal outerWidth = (rect.width() - centerWidth) / outerColumns;

        if (usedColumns[0])
            widths[0] = outerWidth;

        if (usedColumns[2])
            widths[2] = outerWidth;
    }
    else
    {
        widths[1] = rect.width();
    }

    if (outerRows > 0)
    {
        const qreal outerHeight = (rect.height() - centerHeight) / outerRows;

        if (usedRows[0])
            heights[0] = outerHeight;

        if (usedRows[2])
            heights[2] = outerHeight;
    }
    else
    {
        heights[1] = rect.height();
    }

    const qreal x[3] = { rect.left(), rect.left() + widths[0], rect.right() - widths[2] };
    const qreal y[3] = { rect.top(), rect.top() + heights[0], rect.bottom() - heights[2] };

    const int index = static_cast<int>(position);
    const int column = index % 3;
    const int row = index / 3;

    return QRectF(x[column], y[row], widths[column], heights[row]);
}
void C64KeyboardKey::paintLabels(QPainter* ptrPainter, const QRectF& rect) const
{
    if (ptrPainter == nullptr || m_labels.isEmpty() || rect.isEmpty())
        return;

    constexpr qreal SymbolMargin = 0.025;

    const QColor color(30, 30, 30);

    for (const C64KeyboardKeyLabel& label : m_labels)
    {
        const QRectF labelRect = calculateLabelRect(label.position, rect);

        if (labelRect.isEmpty())
            continue;

        if (label.type == C64KeyboardLabelType::Graphic)
        {
            const C64KeyboardSymbolBitmap* ptrBitmap = C64KeyboardSymbolData::bitmap(m_key, label.modifier);

            if (ptrBitmap == nullptr)
                continue;

            const qreal size = qMin(labelRect.width(), labelRect.height());
            const QRectF symbolRect(labelRect.center().x() - size / 2.0, labelRect.center().y() - size / 2.0, size, size);
            const QRectF bitmapRect = symbolRect.adjusted(SymbolMargin, SymbolMargin, -SymbolMargin, -SymbolMargin);

            if (bitmapRect.isEmpty())
                continue;

            ptrPainter->save();

            QPen borderPen(color);
            borderPen.setWidth(0);
            borderPen.setCosmetic(true);

            ptrPainter->setPen(borderPen);
            ptrPainter->setBrush(Qt::NoBrush);
            ptrPainter->drawRect(symbolRect);

            C64KeyboardSymbolPainter::paintBitmap(ptrPainter, *ptrBitmap, bitmapRect, color);

            ptrPainter->restore();
        }
        else if (label.type == C64KeyboardLabelType::Symbol)
        {
            C64KeyboardSymbolPainter::paintSymbol(ptrPainter, label.symbol, labelRect, color);
        }
        else
        {
            ptrPainter->save();

            QFont font = ptrPainter->font();
            font.setBold(true);
            font.setPixelSize(100);
            ptrPainter->setFont(font);

            const QFontMetricsF metrics(font);
            const qreal textWidth = metrics.horizontalAdvance(label.text);
            const qreal textHeight = metrics.height();

            if (textWidth > 0.0 && textHeight > 0.0)
            {
                constexpr qreal TextScaleFactor = 1.15;
                constexpr qreal TextHorizontalMargin = 0.04;
                const qreal availableWidth = qMax(0.0, labelRect.width() - 2.0 * TextHorizontalMargin);
                qreal textScale = qMin(availableWidth / textWidth, labelRect.height() / textHeight) * TextScaleFactor;
                QPointF textCenter = labelRect.center();

                if (label.position == C64KeyboardLabelPosition::TopLeft)
                {
                    textCenter.rx() -= labelRect.width() * 0.08;
                    textCenter.ry() -= labelRect.height() * 0.08;
                }

                constexpr qreal MaximumTextHeight = 0.45;
                textScale = qMin(textScale, MaximumTextHeight / textHeight);

                ptrPainter->setPen(color);
                ptrPainter->translate(textCenter);
                ptrPainter->scale(textScale, textScale);

                const QRectF localTextRect(-textWidth / 2.0, -textHeight / 2.0, textWidth, textHeight);

                ptrPainter->drawText(localTextRect, Qt::AlignCenter, label.text);
            }

            ptrPainter->restore();
        }
    }
}
