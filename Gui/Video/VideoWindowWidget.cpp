#include "VideoWindowWidget.h"

#include <QPainter>
#include <QApplication>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QFocusEvent>


VideoWindowWidget::VideoWindowWidget(QWidget *parent)
    : QWidget{parent}
{
}
VideoWindowWidget::~VideoWindowWidget()
{
}

void VideoWindowWidget::setFrame(const QImage& image)
{
    m_image = image;
    m_bFramePending = true;
    update();
}

void VideoWindowWidget::paintEvent(QPaintEvent* ptrEvent)
{
    Q_UNUSED(ptrEvent);

    QPainter painter(this);

    if (m_image.isNull())
        return;

    QSize imageSize =
        m_image.size();

    imageSize.scale(
        size(),
        Qt::KeepAspectRatio);

    const QRect targetRect(
        (width() - imageSize.width()) / 2,
        (height() - imageSize.height()) / 2,
        imageSize.width(),
        imageSize.height());

    painter.drawImage(
        targetRect,
        m_image);

    if (m_bFramePending)
    {
        m_bFramePending = false;
        emit frameTaken();
    }
}

void VideoWindowWidget::closeEvent(QCloseEvent* ptrEvent)
{
    QApplication::quit();
    QWidget::closeEvent(ptrEvent);
}

void VideoWindowWidget::keyPressEvent(QKeyEvent* ptrEvent)
{
    emit keyPressed(ptrEvent->key(), ptrEvent->nativeScanCode(), ptrEvent->modifiers(), ptrEvent->isAutoRepeat());
}
void VideoWindowWidget::keyReleaseEvent(QKeyEvent* ptrEvent)
{
    emit keyReleased(ptrEvent->key(), ptrEvent->nativeScanCode(), ptrEvent->modifiers(), ptrEvent->isAutoRepeat());
}

void VideoWindowWidget::changeEvent(QEvent* ptrEvent)
{
    if (ptrEvent->type() == QEvent::ActivationChange && !isActiveWindow())
    {
        emit inputDeactivated();
    }
    QWidget::changeEvent(ptrEvent);
}
