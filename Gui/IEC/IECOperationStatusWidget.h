#pragma once

#include <QWidget>
#include <QTimer>
#include <QString>


class IECOperationStatusWidget : public QWidget
{
    Q_OBJECT

public:
    enum class State
    {
        Idle,
        Pending,
        Success,
        Failed
    };

    explicit IECOperationStatusWidget(QWidget* parent = nullptr);
    virtual ~IECOperationStatusWidget();

    void setState(State state, const QString& message = QString());
    State state() const;

    void setSuccessDuration(int durationMs);
    int successDuration() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* ptrEvent) override;

private:
    void advanceAnimation();
    void drawSpinner(QPainter& painter);
    void drawSuccess(QPainter& painter);
    void drawFailed(QPainter& painter);

private:
    State m_state = State::Idle;

    QTimer m_animationTimer;
    QTimer m_successTimer;

    int m_animationStep = 0;
    int m_successDuration = 1500;

    QString m_message;
};
