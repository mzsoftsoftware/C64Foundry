#pragma once

#include <QWidget>
#include <QColor>
#include <QElapsedTimer>
#include <QTimer>


class IECActivityLEDWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IECActivityLEDWidget(QWidget* parent = nullptr);
    virtual ~IECActivityLEDWidget();

    void setLEDCount(int count);
    int ledCount() const;

    void setLEDColor(int index, const QColor& color);
    QColor ledColor(int index) const;

    void setFadeDuration(int durationMs);
    int fadeDuration() const;

    void triggerActivity(int index = 0);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* ptrEvent) override;

private slots:
    void updateAnimation();

private:
    struct LED
    {
        QColor color = QColor(45, 180, 75);
        qint64 lastActivity = -1;
    };

    void drawLED(QPainter& painter, const QPointF& center, const QColor& color, qreal brightness);
    qreal brightness(int index) const;

private:
    LED m_leds[3];

    int m_ledCount = 1;
    int m_fadeDuration = 1500;

    QElapsedTimer m_elapsedTimer;
    QTimer m_animationTimer;
};
