#pragma once

#include <QThread>

class VideoWorker;


class VideoThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoThread(VideoWorker* ptrWorker, QObject* parent = nullptr);
    virtual ~VideoThread();

protected:
    void run() override;

private:
    VideoWorker* m_ptrWorker = nullptr;
};
