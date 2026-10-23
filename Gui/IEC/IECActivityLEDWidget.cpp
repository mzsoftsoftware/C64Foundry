#include "IECActivityLEDWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QtMath>


IECActivityLEDWidget::IECActivityLEDWidget(QWidget* parent)
    : QWidget(parent)
{
    m_elapsedTimer.start();

    m_animationTimer.setInterval(30);
    m_animationTimer.setTimerType(Qt::PreciseTimer);

    connect(&m_animationTimer, &QTimer::timeout, this, &IECActivityLEDWidget::updateAnimation);
}


IECActivityLEDWidget::~IECActivityLEDWidget()
{
}


void IECActivityLEDWidget::setLEDCount(int count)
{
    count = qBound(1, count, 3);

    if (m_ledCount == count)
        return;

    m_ledCount = count;

    updateGeometry();
    update();
}


int IECActivityLEDWidget::ledCount() const
{
    return m_ledCount;
}


void IECActivityLEDWidget::setLEDColor(int index, const QColor& color)
{
    if (index < 0 || index >= 3)
        return;

    m_leds[index].color = color;
    update();
}


QColor IECActivityLEDWidget::ledColor(int index) const
{
    if (index < 0 || index >= 3)
        return QColor();

    return m_leds[index].color;
}


void IECActivityLEDWidget::setFadeDuration(int durationMs)
{
    m_fadeDuration = qMax(1, durationMs);
    updateAnimation();
}


int IECActivityLEDWidget::fadeDuration() const
{
    return m_fadeDuration;
}


void IECActivityLEDWidget::triggerActivity(int index)
{
    if (index < 0 || index >= m_ledCount)
        return;

    m_leds[index].lastActivity = m_elapsedTimer.elapsed();

    if (!m_animationTimer.isActive())
        m_animationTimer.start();

    update();
}


QSize IECActivityLEDWidget::sizeHint() const
{
    return QSize(24, 24);
}


QSize IECActivityLEDWidget::minimumSizeHint() const
{
    return QSize(20, 20);
}


qreal IECActivityLEDWidget::brightness(int index) const
{
    if (index < 0 || index >= m_ledCount)
        return 0.0;

    if (m_leds[index].lastActivity < 0)
        return 0.0;

    const qint64 elapsed = m_elapsedTimer.elapsed() - m_leds[index].lastActivity;

    if (elapsed >= m_fadeDuration)
        return 0.0;

    const qreal progress = static_cast<qreal>(elapsed) / static_cast<qreal>(m_fadeDuration);

    // Quadratischer Verlauf: zunächst hell, danach sanftes Ausblenden.
    // Die ersten 30 % bleibt die LED vollständig hell.
    if (progress <= 0.3)
        return 1.0;

    // Anschließend gleichmäßig abdunkeln.
    return 1.0 - (progress - 0.3) / 0.7;
}


void IECActivityLEDWidget::updateAnimation()
{
    bool active = false;

    for (int index = 0; index < m_ledCount; ++index)
    {
        if (brightness(index) > 0.0)
        {
            active = true;
            break;
        }
    }

    if (!active)
        m_animationTimer.stop();

    update();
}


void IECActivityLEDWidget::paintEvent(QPaintEvent* ptrEvent)
{
    Q_UNUSED(ptrEvent)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Einheitliches Koordinatensystem für alle LED-Anzahlen.
    painter.translate(width() / 2.0, height() / 2.0);

    const qreal scale = qMin(width() / 24.0, height() / 24.0);
    painter.scale(scale, scale);

    for (int index = 0; index < m_ledCount; ++index)
    {
        const qreal spacing = 7.0;
        const qreal y = (static_cast<qreal>(index) - (m_ledCount - 1) / 2.0) * spacing;

        drawLED(painter, QPointF(0.0, y), m_leds[index].color, brightness(index));
    }
}


void IECActivityLEDWidget::drawLED(QPainter& painter, const QPointF& center, const QColor& color, qreal brightness)
{
    painter.save();
    painter.translate(center);

    // Dunkler äußerer LED-Rahmen.
    painter.setPen(QPen(palette().color(QPalette::Mid), 0.7));
    painter.setBrush(palette().color(QPalette::Dark));
    painter.drawEllipse(QPointF(0.0, 0.0), 3.1, 3.1);

    // LED-Farbe abhängig von der aktuellen Helligkeit.
    const QColor background = palette().color(QPalette::Dark);

    QColor ledColor;
    ledColor.setRedF(background.redF() * (1.0 - brightness) + color.redF() * brightness);
    ledColor.setGreenF(background.greenF() * (1.0 - brightness) + color.greenF() * brightness);
    ledColor.setBlueF(background.blueF() * (1.0 - brightness) + color.blueF() * brightness);

    painter.setPen(Qt::NoPen);
    painter.setBrush(ledColor);
    painter.drawEllipse(QPointF(0.0, 0.0), 2.5, 2.5);

    // Kleiner Lichtreflex.
    QColor highlight = Qt::white;
    highlight.setAlphaF(0.12 + brightness * 0.35);

    painter.setBrush(highlight);
    painter.drawEllipse(QPointF(-0.8, -0.9), 0.7, 0.7);

    painter.restore();
}
