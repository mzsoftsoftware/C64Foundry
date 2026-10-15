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
