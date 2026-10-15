#pragma once

#include <QColor>
#include <QPainter>
#include <QRectF>

#include "C64KeyboardSymbolData.h"


class C64KeyboardSymbolPainter
{
public:
    static void paintBitmap(QPainter* ptrPainter, const C64KeyboardSymbolBitmap& bitmap, const QRectF& rect, const QColor& color);
    static void paintSymbol(QPainter* ptrPainter, C64KeyboardSymbol symbol, const QRectF& rect, const QColor& color);
};
