#include "C64KeyboardSymbolPainter.h"

#include <QFontMetricsF>


void C64KeyboardSymbolPainter::paintBitmap(QPainter* ptrPainter, const C64KeyboardSymbolBitmap& bitmap, const QRectF& rect, const QColor& color)
{
    if (ptrPainter == nullptr || rect.isEmpty())
    {
        return;
    }

    ptrPainter->save();

    ptrPainter->setPen(Qt::NoPen);
    ptrPainter->setBrush(color);

    const qreal pixelWidth = rect.width() / 8.0;
    const qreal pixelHeight = rect.height() / 8.0;

    for (int y = 0; y < 8; ++y)
    {
        const quint8 row = bitmap.rows[y];

        for (int x = 0; x < 8; ++x)
        {
            if ((row & (0x80 >> x)) == 0)
            {
                continue;
            }

            const QRectF pixelRect(rect.left() + x * pixelWidth, rect.top() + y * pixelHeight, pixelWidth, pixelHeight);

            ptrPainter->drawRect(pixelRect);
        }
    }

    ptrPainter->restore();
}

void C64KeyboardSymbolPainter::paintSymbol(QPainter* ptrPainter, const C64KeyboardSymbol symbol, const QRectF& rect, const QColor& color)
{
    if (ptrPainter == nullptr || rect.isEmpty())
        return;

    switch (symbol)
    {
    case C64KeyboardSymbol::ArrowUp:
    case C64KeyboardSymbol::ArrowDown:
    case C64KeyboardSymbol::ArrowLeft:
    case C64KeyboardSymbol::ArrowRight:
    case C64KeyboardSymbol::DoubleArrowUp:
    case C64KeyboardSymbol::DoubleArrowDown:
    case C64KeyboardSymbol::DoubleArrowLeft:
    case C64KeyboardSymbol::DoubleArrowRight:
        paintArrow(ptrPainter, rect, symbol, color);
        break;

    case C64KeyboardSymbol::ColorBlack:
        paintSquare(ptrPainter, rect, QColor(0, 0, 0), true);
        break;

    case C64KeyboardSymbol::ColorWhite:
        paintSquare(ptrPainter, rect, QColor(255, 255, 255), true);
        break;

    case C64KeyboardSymbol::ColorRed:
        paintSquare(ptrPainter, rect, QColor(136, 0, 0), true);
        break;

    case C64KeyboardSymbol::ColorCyan:
        paintSquare(ptrPainter, rect, QColor(170, 255, 238), true);
        break;

    case C64KeyboardSymbol::ColorPurple:
        paintSquare(ptrPainter, rect, QColor(204, 68, 204), true);
        break;

    case C64KeyboardSymbol::ColorGreen:
        paintSquare(ptrPainter, rect, QColor(0, 204, 85), true);
        break;

    case C64KeyboardSymbol::ColorBlue:
        paintSquare(ptrPainter, rect, QColor(0, 0, 170), true);
        break;

    case C64KeyboardSymbol::ColorYellow:
        paintSquare(ptrPainter, rect, QColor(238, 238, 119), true);
        break;

    case C64KeyboardSymbol::ReverseOn:
        paintSquare(ptrPainter, rect, color, true, true);
        break;

    case C64KeyboardSymbol::ReverseOff:
        paintSquare(ptrPainter, rect, QColor(200, 190, 170), true, true);
        break;
    }
}

void C64KeyboardSymbolPainter::paintArrow(QPainter* ptrPainter, const QRectF& rect, const C64KeyboardSymbol symbol, const QColor& color)
{
    if (ptrPainter == nullptr || rect.isEmpty())
        return;

    const bool doubleArrow = symbol == C64KeyboardSymbol::DoubleArrowUp || symbol == C64KeyboardSymbol::DoubleArrowDown || symbol == C64KeyboardSymbol::DoubleArrowLeft || symbol == C64KeyboardSymbol::DoubleArrowRight;

    const bool down = symbol == C64KeyboardSymbol::ArrowDown || symbol == C64KeyboardSymbol::DoubleArrowDown;
    const bool left = symbol == C64KeyboardSymbol::ArrowLeft || symbol == C64KeyboardSymbol::DoubleArrowLeft;
    const bool right = symbol == C64KeyboardSymbol::ArrowRight || symbol == C64KeyboardSymbol::DoubleArrowRight;

    const bool singleArrowUp = symbol == C64KeyboardSymbol::ArrowUp;
    const bool singleArrowLeft = symbol == C64KeyboardSymbol::ArrowLeft;

    const qreal size = qMin(rect.width(), rect.height());
    const qreal lineWidth = doubleArrow || singleArrowUp ? 7.0 : 5.0;
    const qreal arrowHeadWidth = singleArrowUp ? 28.0 : 30.0;
    const qreal arrowHeadBottom = singleArrowUp ? -13.0 : -8.0;

    ptrPainter->save();
    ptrPainter->setRenderHint(QPainter::Antialiasing, true);

    ptrPainter->translate(rect.center());

    if (singleArrowLeft)
        ptrPainter->scale(size / 100.0, size / 100.0 * 0.85);
    else
        ptrPainter->scale(size / 100.0, size / 100.0);

    if (down)
        ptrPainter->rotate(180.0);
    else if (left)
        ptrPainter->rotate(-90.0);
    else if (right)
        ptrPainter->rotate(90.0);

    QPen pen(color);
    pen.setWidthF(lineWidth);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);

    ptrPainter->setPen(pen);
    ptrPainter->setBrush(Qt::NoBrush);

    // Pfeilkopf
    ptrPainter->drawLine(QPointF(-arrowHeadWidth, arrowHeadBottom), QPointF(0.0, -28.0));
    ptrPainter->drawLine(QPointF(0.0, -28.0), QPointF(arrowHeadWidth, arrowHeadBottom));

    // Schaft
    if (doubleArrow)
    {
        ptrPainter->drawLine(QPointF(-10.0, -23.0), QPointF(-10.0, 25.0));
        ptrPainter->drawLine(QPointF(10.0, -23.0), QPointF(10.0, 25.0));
    }
    else
    {
        ptrPainter->drawLine(QPointF(0.0, -23.0), QPointF(0.0, 25.0));
    }

    ptrPainter->restore();
}

void C64KeyboardSymbolPainter::paintSquare(QPainter* ptrPainter, const QRectF& rect, const QColor& fillColor, const bool filled, const bool reverseCharacter)
{
    if (ptrPainter == nullptr || rect.isEmpty())
        return;

    const QColor borderColor(30, 30, 30);
    const QColor lightColor(200, 190, 170);

    const qreal size = qMin(rect.width(), rect.height());
    const QRectF squareRect(rect.center().x() - size / 2.0, rect.center().y() - size / 2.0, size, size);

    ptrPainter->save();
    ptrPainter->setRenderHint(QPainter::Antialiasing, true);

    QPen pen(borderColor);
    pen.setWidthF(qMax(0.015, size * 0.06));
    pen.setJoinStyle(Qt::MiterJoin);

    ptrPainter->setPen(pen);
    ptrPainter->setBrush(filled ? QBrush(fillColor) : QBrush(Qt::NoBrush));
    ptrPainter->drawRect(squareRect);

    if (reverseCharacter)
    {
        const bool darkBackground = fillColor.lightness() < 128;
        const QColor textColor = darkBackground ? lightColor : borderColor;

        QFont font = ptrPainter->font();
        font.setBold(true);
        font.setPixelSize(100);

        const QFontMetricsF metrics(font);
        const qreal textWidth = metrics.horizontalAdvance(QStringLiteral("A"));
        const qreal textHeight = metrics.height();

        if (textWidth > 0.0 && textHeight > 0.0)
        {
            const qreal textScale = qMin(squareRect.width() * 0.90 / textWidth, squareRect.height() * 0.90 / textHeight);

            ptrPainter->setFont(font);
            ptrPainter->setPen(textColor);

            ptrPainter->translate(squareRect.center());
            ptrPainter->scale(textScale, textScale);

            ptrPainter->drawText(QRectF(-textWidth / 2.0, -textHeight / 2.0, textWidth, textHeight), Qt::AlignCenter, QStringLiteral("A"));
        }
    }

    ptrPainter->restore();
}
