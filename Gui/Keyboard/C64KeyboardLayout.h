#pragma once

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

#include "C64KeyboardKey.h"


class C64KeyboardLayout
{
public:
    C64KeyboardLayout();

    QRectF bounds() const                           { return m_bounds; }
    const QVector<C64KeyboardKey>& keys() const     { return m_keys; }
    QVector<C64KeyboardKey>& keys()                 { return m_keys; }
    C64KeyboardKey* keyAt(const QPointF& position);
    C64KeyboardKey* keyById(C64Key key);

    const C64KeyboardKey* keyAt(const QPointF& position) const;

private:
    C64KeyboardKey* addKey(const C64Key key, const QString& qstrLabel, const qreal x, const qreal y, const qreal width = 1.0);
    void updateBounds();

private:
    QVector<C64KeyboardKey> m_keys;
    QRectF m_bounds;
};
