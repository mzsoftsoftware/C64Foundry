#pragma once

#include <QWidget>
#include <QImage>

class QKeyEvent;
class QEvent;


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
    void keyPressed(int key, quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void keyReleased(int key, quint32 nativeScanCode, Qt::KeyboardModifiers modifiers, bool autoRepeat);
    void inputDeactivated();

protected:
    bool event(QEvent* ptrEvent) override;
    void paintEvent(QPaintEvent* ptrEvent) override;
    void closeEvent(QCloseEvent* ptrEvent) override;
    void keyPressEvent(QKeyEvent* ptrEvent) override;
    void keyReleaseEvent(QKeyEvent* ptrEvent) override;
    void changeEvent(QEvent* ptrEvent) override;

private:
    QImage m_image;
    bool m_bFramePending = false;
};
