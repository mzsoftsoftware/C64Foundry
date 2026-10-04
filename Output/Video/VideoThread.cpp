#include "VideoThread.h"

#include "Output/Video/VideoWorker.h"


VideoThread::VideoThread(VideoWorker* ptrWorker, QObject* parent)
    : QThread{parent}
    , m_ptrWorker{ptrWorker}
{
}

VideoThread::~VideoThread()
{
}

void VideoThread::run()
{
    m_ptrWorker->run();
}
