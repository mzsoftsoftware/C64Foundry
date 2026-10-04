#include "VideoController.h"

#include <QDebug>

#include "Output/Video/VideoThread.h"
#include "Output/Video/VideoWorker.h"
#include "Gui/Video/VideoWindowWidget.h"


VideoController::VideoController(C64Machine* ptrMachine, QObject* parent)
    : QObject{parent}
{
    qDebug() << "VideoController: created" << QThread::currentThreadId();

    m_ptrWorker = new VideoWorker(ptrMachine);
    m_ptrThread = new VideoThread(m_ptrWorker, this);

    m_ptrThread->start();
}

VideoController::~VideoController()
{
    qDebug() << "VideoController: shutting down";

    m_ptrWorker->requestShutdown();
    m_ptrThread->wait();

    delete m_ptrWorker;

    qDebug() << "VideoController: destroyed";
}

void VideoController::setVideoWidget(VideoWindowWidget* ptrVideoWidget)
{
    connect(m_ptrWorker, &VideoWorker::frameReady, ptrVideoWidget, &VideoWindowWidget::setFrame, Qt::QueuedConnection);
    connect(ptrVideoWidget, &VideoWindowWidget::frameTaken, m_ptrWorker, &VideoWorker::frameTaken, Qt::DirectConnection);
}
