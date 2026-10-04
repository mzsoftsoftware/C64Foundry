#pragma once

#include <QWidget>
#include <QImage>


class VideoWindowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWindowWidget(QWidget *parent = nullptr);
    virtual ~VideoWindowWidget();

public slots:
    void setFrame(const QImage& image);

signals:
    void frameTaken();

protected:
    void paintEvent(QPaintEvent* ptrEvent) override;

private:
    QImage m_image;
};
