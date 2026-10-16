#include "C64KeyboardSymbolPainter.h"


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

    const bool doubleArrow = symbol == C64KeyboardSymbol::DoubleArrowUp || symbol == C64KeyboardSymbol::DoubleArrowDown || symbol == C64KeyboardSymbol::DoubleArrowLeft || symbol == C64KeyboardSymbol::DoubleArrowRight;

    const bool up = symbol == C64KeyboardSymbol::ArrowUp || symbol == C64KeyboardSymbol::DoubleArrowUp;
    const bool down = symbol == C64KeyboardSymbol::ArrowDown || symbol == C64KeyboardSymbol::DoubleArrowDown;
    const bool left = symbol == C64KeyboardSymbol::ArrowLeft || symbol == C64KeyboardSymbol::DoubleArrowLeft;

    ptrPainter->save();
    ptrPainter->setRenderHint(QPainter::Antialiasing, true);

    const qreal size = qMin(rect.width(), rect.height());
    const QRectF symbolRect(rect.center().x() - size / 2.0, rect.center().y() - size / 2.0, size, size);

    ptrPainter->translate(symbolRect.center());
    ptrPainter->scale(size / 100.0, size / 100.0);

    if (down)
        ptrPainter->rotate(180.0);
    else if (left)
        ptrPainter->rotate(-90.0);
    else if (!up)
        ptrPainter->rotate(90.0);

    QPen pen(color);
    pen.setWidthF(5.0);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);

    ptrPainter->setPen(pen);
    ptrPainter->setBrush(Qt::NoBrush);

    // Pfeilkopf: flacher, aber weiterhin breit
    ptrPainter->drawLine(QPointF(-30.0, -8.0), QPointF(0.0, -28.0));
    ptrPainter->drawLine(QPointF(0.0, -28.0), QPointF(30.0, -8.0));

    // Schaft: kürzer und mit größerem Abstand
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
