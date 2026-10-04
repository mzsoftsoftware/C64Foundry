#include "VideoWorker.h"

#include <QThread>

#include "C64/C64Machine.h"


VideoWorker::VideoWorker(C64Machine* ptrMachine, QObject* parent)
    : QObject{parent}
    , m_ptrMachine{ptrMachine}
{
}
VideoWorker::~VideoWorker()
{
}

void VideoWorker::run()
{
    while (true)
    {
        bool bShutdownRequested = false;
        {
            QMutexLocker locker(&m_mutex);
            bShutdownRequested = m_bShutdownRequested;
        }

        if (bShutdownRequested)
            break;

        const quint8* ptrFrame = m_ptrMachine->acquireVideoFrame();
        if(ptrFrame == nullptr)
        {
            QThread::msleep(10);
            continue;
        }

        //
        // Frame processing will be added here.
        //
    }
}

void VideoWorker::requestShutdown()
{
    QMutexLocker locker(&m_mutex);
    m_bShutdownRequested = true;
}
