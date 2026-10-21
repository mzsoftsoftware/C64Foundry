#include "IECOperationStatusWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPalette>
#include <QPen>
#include <QtMath>


IECOperationStatusWidget::IECOperationStatusWidget(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(20, 20);

    m_animationTimer.setInterval(80);
    m_animationTimer.setTimerType(Qt::PreciseTimer);

    connect(&m_animationTimer, &QTimer::timeout, this, &IECOperationStatusWidget::advanceAnimation);

    m_successTimer.setSingleShot(true);

    connect(&m_successTimer, &QTimer::timeout, this, [this]()
            {
                if (m_state == State::Success)
                    setState(State::Idle);
            });
}


IECOperationStatusWidget::~IECOperationStatusWidget()
{
}


void IECOperationStatusWidget::setState(State state, const QString& message)
{
    m_animationTimer.stop();
    m_successTimer.stop();

    m_state = state;
    m_message = message;
    m_animationStep = 0;

    switch (m_state)
    {
    case State::Idle:
        setToolTip(QString());
        break;

    case State::Pending:
        setToolTip(m_message.isEmpty() ? tr("Operation pending") : m_message);
        m_animationTimer.start();
        break;

    case State::Success:
        setToolTip(m_message.isEmpty() ? tr("Operation successful") : m_message);

        if (m_successDuration > 0)
            m_successTimer.start(m_successDuration);

        break;

    case State::Failed:
        setToolTip(m_message.isEmpty() ? tr("Operation failed") : m_message);
        break;
    }

    update();
}


IECOperationStatusWidget::State IECOperationStatusWidget::state() const
{
    return m_state;
}


void IECOperationStatusWidget::setSuccessDuration(int durationMs)
{
    m_successDuration = qMax(0, durationMs);

    if (m_state == State::Success)
    {
        m_successTimer.stop();

        if (m_successDuration > 0)
            m_successTimer.start(m_successDuration);
    }
}


int IECOperationStatusWidget::successDuration() const
{
    return m_successDuration;
}


QSize IECOperationStatusWidget::sizeHint() const
{
    return QSize(20, 20);
}


void IECOperationStatusWidget::advanceAnimation()
{
    m_animationStep = (m_animationStep + 1) % 12;
    update();
}


void IECOperationStatusWidget::paintEvent(QPaintEvent* ptrEvent)
{
    Q_UNUSED(ptrEvent)

    if (m_state == State::Idle)
        return;

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);

    // Einheitliches Koordinatensystem unabhängig von der Widgetgröße.
    painter.translate(width() / 2.0, height() / 2.0);

    const qreal scale = qMin(width(), height()) / 20.0;
    painter.scale(scale, scale);

    switch (m_state)
    {
    case State::Idle:
        break;

    case State::Pending:
        drawSpinner(painter);
        break;

    case State::Success:
        drawSuccess(painter);
        break;

    case State::Failed:
        drawFailed(painter);
        break;
    }
}


void IECOperationStatusWidget::drawSpinner(QPainter& painter)
{
    const QColor baseColor = palette().color(QPalette::Text);

    painter.setPen(Qt::NoPen);

    for (int index = 0; index < 12; ++index)
    {
        const int distance = (index - m_animationStep + 12) % 12;

        QColor color = baseColor;

        // Der führende Punkt ist vollständig sichtbar.
        // Die nachfolgenden Punkte bilden den Schweif.
        const int alpha = 255 - distance * 18;
        color.setAlpha(qMax(40, alpha));

        painter.setBrush(color);

        const qreal angle = (index * 30.0 - 90.0) * 3.14159265358979323846 / 180.0;

        const qreal x = qCos(angle) * 6.5;
        const qreal y = qSin(angle) * 6.5;

        painter.drawEllipse(QPointF(x, y), 1.5, 1.5);
    }
}


void IECOperationStatusWidget::drawSuccess(QPainter& painter)
{
    const QColor color(45, 160, 75);

    painter.setPen(QPen(color, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    // Häkchen
    QPainterPath path;
    path.moveTo(-6.0, 0.0);
    path.lineTo(-1.5, 4.5);
    path.lineTo(6.5, -5.0);

    painter.drawPath(path);
}


void IECOperationStatusWidget::drawFailed(QPainter& painter)
{
    const QColor color(210, 65, 55);

    painter.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    // Warn-Dreieck
    QPainterPath triangle;
    triangle.moveTo(0.0, -7.5);
    triangle.lineTo(8.0, 6.5);
    triangle.lineTo(-8.0, 6.5);
    triangle.closeSubpath();

    painter.drawPath(triangle);

    // Ausrufezeichen
    painter.setPen(QPen(color, 2.0, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(0.0, -3.0), QPointF(0.0, 1.5));

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QPointF(0.0, 4.0), 1.0, 1.0);
}
