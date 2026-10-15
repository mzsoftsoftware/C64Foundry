#include "C64KeyboardWidget.h"

#include <QPaintEvent>
#include <QPainter>


C64KeyboardWidget::C64KeyboardWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}
C64KeyboardWidget::~C64KeyboardWidget()
{
}

void C64KeyboardWidget::setMode(const Mode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    setHoveredKey(nullptr);
    update();
}

void C64KeyboardWidget::setKeyboardKeyPressed(const C64Key key, const bool bPressed)
{
    if (m_mode != Mode::Input)
        return;
    C64KeyboardKey* ptrKey = m_layout.keyById(key);
    if (ptrKey != nullptr && ptrKey->setKeyboardPressed(bPressed))
        updateKey(ptrKey);
}
void C64KeyboardWidget::setModifiers(const C64KeyboardModifiers& modifiers)
{
    if (m_mode != Mode::Input)
        return;
    for (C64KeyboardKey& key : m_layout.keys())
    {
        if (key.setModifiers(modifiers))
            updateKey(&key);
    }
}


QTransform C64KeyboardWidget::keyboardTransform() const
{
    constexpr qreal KeyboardMargin = 0.2;
    const QRectF bounds = m_layout.bounds().adjusted(-KeyboardMargin, -KeyboardMargin, KeyboardMargin, KeyboardMargin);
    if (bounds.isEmpty() || width() <= 0 || height() <= 0)
        return QTransform();

    const qreal scale = qMin(qreal(width()) / bounds.width(), qreal(height()) / bounds.height());
    const qreal offsetX = (width() - bounds.width() * scale) / 2.0;
    const qreal offsetY = (height() - bounds.height() * scale) / 2.0;

    QTransform transform;
    transform.translate(offsetX, offsetY);
    transform.scale(scale, scale);
    transform.translate(-bounds.left(), -bounds.top());

    return transform;
}
QPointF C64KeyboardWidget::mapToKeyboard(const QPointF& position) const
{
    bool bInvertible = false;
    const QTransform inverse = keyboardTransform().inverted(&bInvertible);
    if (!bInvertible)
        return QPointF();

    return inverse.map(position);
}

void C64KeyboardWidget::updateKey(const C64KeyboardKey* ptrKey)
{
    if (ptrKey == nullptr)
        return;

    const QRectF widgetRect = keyboardTransform().mapRect(ptrKey->rect());

    update(widgetRect.toAlignedRect().adjusted(-2, -2, 2, 2));
}
void C64KeyboardWidget::setHoveredKey(C64KeyboardKey* ptrKey)
{
    if (m_ptrHoveredKey == ptrKey)
        return;
    if (m_ptrHoveredKey != nullptr)
    {
        if (m_ptrHoveredKey->setHovered(false))
            updateKey(m_ptrHoveredKey);
    }

    m_ptrHoveredKey = ptrKey;
    if (m_ptrHoveredKey != nullptr)
    {
        if (m_ptrHoveredKey->setHovered(true))
            updateKey(m_ptrHoveredKey);
    }
}

void C64KeyboardWidget::paintEvent(QPaintEvent* ptrEvent)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(ptrEvent->rect(), QColor(70, 65, 60));
    painter.setTransform(keyboardTransform());

    for (const C64KeyboardKey& key : m_layout.keys())
        key.paint(&painter);
}

void C64KeyboardWidget::mouseMoveEvent(QMouseEvent* ptrEvent)
{
    const QPointF keyboardPosition = mapToKeyboard(ptrEvent->position());
    setHoveredKey(m_layout.keyAt(keyboardPosition));
    QWidget::mouseMoveEvent(ptrEvent);
}

void C64KeyboardWidget::leaveEvent(QEvent* ptrEvent)
{
    setHoveredKey(nullptr);
    QWidget::leaveEvent(ptrEvent);
}
