#pragma once

#include <QObject>

class VideoThread;
class VideoWorker;
class VideoWindowWidget;
class VideoFrameSource;


class VideoController : public QObject
{
    Q_OBJECT
public:
    explicit VideoController(VideoFrameSource* ptrSource, QObject* parent);
    virtual ~VideoController();

    // Operations
    void setVideoWidget(VideoWindowWidget* ptrVideoWidget);

private:
    VideoWorker* m_ptrWorker = nullptr;
    VideoThread* m_ptrThread = nullptr;
};
