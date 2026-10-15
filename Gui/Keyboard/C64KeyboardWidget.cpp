#include "C64KeyboardWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>


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
    cancelMouseKey();
    releaseTemporaryModifiers();
    C64KeyboardKey* ptrShiftLock = m_layout.keyById(C64Key::ShiftLock);
    if (ptrShiftLock != nullptr && ptrShiftLock->isLatched())
        setModifierLatched(ptrShiftLock, false);
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
void C64KeyboardWidget::resetKeyboardState()
{
    m_ptrMousePressedKey = nullptr;
    setHoveredKey(nullptr);
    for (C64KeyboardKey& key : m_layout.keys())
    {
        if (key.setMousePressed(false))
            updateKey(&key);
        if (key.setLatched(false))
            updateKey(&key);
        if (key.setKeyboardPressed(false))
            updateKey(&key);
    }
}

bool C64KeyboardWidget::isModifierKey(const C64Key key) const
{
    switch (key)
    {
    case C64Key::LeftShift:
    case C64Key::RightShift:
    case C64Key::Control:
    case C64Key::Commodore:
    case C64Key::ShiftLock:
        return true;
    default:
        return false;
    }
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
void C64KeyboardWidget::setModifierLatched(C64KeyboardKey* ptrKey, const bool bLatched)
{
    if (ptrKey == nullptr || !isModifierKey(ptrKey->key()))
        return;
    if (!ptrKey->setLatched(bLatched))
        return;
    updateKey(ptrKey);
    if (bLatched)
        emit keyPressed(ptrKey->key());
    else
        emit keyReleased(ptrKey->key());
}
void C64KeyboardWidget::releaseTemporaryModifiers()
{
    for (C64KeyboardKey& key : m_layout.keys())
    {
        if (key.key() == C64Key::ShiftLock)
            continue;
        if (isModifierKey(key.key()) && key.isLatched())
            setModifierLatched(&key, false);
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

bool C64KeyboardWidget::event(QEvent* ptrEvent)
{
    if (ptrEvent->type() == QEvent::UngrabMouse)
        cancelMouseKey();
    return QWidget::event(ptrEvent);
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

void C64KeyboardWidget::leaveEvent(QEvent* ptrEvent)
{
    setHoveredKey(nullptr);
    QWidget::leaveEvent(ptrEvent);
}

void C64KeyboardWidget::mouseMoveEvent(QMouseEvent* ptrEvent)
{
    const QPointF keyboardPosition = mapToKeyboard(ptrEvent->position());
    setHoveredKey(m_layout.keyAt(keyboardPosition));
    QWidget::mouseMoveEvent(ptrEvent);
}
void C64KeyboardWidget::mousePressEvent(QMouseEvent* ptrEvent)
{
    if (m_mode != Mode::Input || ptrEvent->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(ptrEvent);
        return;
    }
    if (m_ptrMousePressedKey != nullptr)
    {
        ptrEvent->accept();
        return;
    }
    const QPointF keyboardPosition = mapToKeyboard(ptrEvent->position());
    C64KeyboardKey* ptrKey = m_layout.keyAt(keyboardPosition);
    if (ptrKey == nullptr)
    {
        QWidget::mousePressEvent(ptrEvent);
        return;
    }
    m_ptrMousePressedKey = ptrKey;
    if (ptrKey->setMousePressed(true))
        updateKey(ptrKey);
    if (!isModifierKey(ptrKey->key()))
        emit keyPressed(ptrKey->key());
    ptrEvent->accept();
}
void C64KeyboardWidget::mouseReleaseEvent(QMouseEvent* ptrEvent)
{
    if (ptrEvent->button() != Qt::LeftButton || m_ptrMousePressedKey == nullptr)
    {
        QWidget::mouseReleaseEvent(ptrEvent);
        return;
    }
    releaseMouseKey();
    ptrEvent->accept();
}
void C64KeyboardWidget::releaseMouseKey()
{
    if (m_ptrMousePressedKey == nullptr)
        return;
    C64KeyboardKey* ptrKey = m_ptrMousePressedKey;
    m_ptrMousePressedKey = nullptr;
    if (ptrKey->setMousePressed(false))
        updateKey(ptrKey);
    if (isModifierKey(ptrKey->key()))
    {
        setModifierLatched(ptrKey, !ptrKey->isLatched());
    }
    else
    {
        emit keyReleased(ptrKey->key());
        releaseTemporaryModifiers();
    }
}
void C64KeyboardWidget::cancelMouseKey()
{
    if (m_ptrMousePressedKey == nullptr)
        return;
    C64KeyboardKey* ptrKey = m_ptrMousePressedKey;
    m_ptrMousePressedKey = nullptr;
    if (ptrKey->setMousePressed(false))
        updateKey(ptrKey);
    if (!isModifierKey(ptrKey->key()))
        emit keyReleased(ptrKey->key());
}
