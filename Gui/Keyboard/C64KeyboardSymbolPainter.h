#pragma once

#include <QColor>
#include <QPainter>
#include <QRectF>

#include "C64KeyboardSymbolData.h"
#include "C64KeyboardSymbol.h"


class C64KeyboardSymbolPainter
{
public:
    static void paintBitmap(QPainter* ptrPainter, const C64KeyboardSymbolBitmap& bitmap, const QRectF& rect, const QColor& color);
    static void paintSymbol(QPainter* ptrPainter, C64KeyboardSymbol symbol, const QRectF& rect, const QColor& color);

private:
    static void paintArrow(QPainter* ptrPainter, const QRectF& rect, C64KeyboardSymbol symbol, const QColor& color);
    static void paintSquare(QPainter* ptrPainter, const QRectF& rect, const QColor& fillColor, bool filled, bool reverseCharacter = false);
};