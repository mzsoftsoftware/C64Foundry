#include "VideoWindowWidget.h"

#include <QPainter>


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
    update();
    emit frameTaken();
}

void VideoWindowWidget::paintEvent(QPaintEvent* ptrEvent)
{
    Q_UNUSED(ptrEvent);
    QPainter painter(this);
    if (m_image.isNull())
        return;
    painter.drawImage(rect(), m_image);
}