#include "EmulatorController.h"

#include <QDebug>

#include "Emulator/EmulatorThread.h"
#include "Emulator/EmulatorWorker.h"


EmulatorController::EmulatorController(QObject *parent)
    : QObject{parent}
{
    qDebug() << "EmulatorController: created" << QThread::currentThreadId();

    m_ptrWorker = new EmulatorWorker();
    m_ptrThread = new EmulatorThread(m_ptrWorker, this);

    connect(m_ptrWorker, &EmulatorWorker::runningChanged, this, &EmulatorController::runningChanged);
    connect(m_ptrWorker, &EmulatorWorker::statisticsChanged, this, &EmulatorController::statisticsChanged);

    m_ptrThread->start();
}
EmulatorController::~EmulatorController()
{
    qDebug() << "EmulatorController: shutting down";

    m_ptrWorker->requestShutdown();
    m_ptrThread->wait();

    delete m_ptrWorker;
    qDebug() << "EmulatorController: destroyed";
}

void EmulatorController::start()
{
    qDebug() << "EmulatorController: start";
    m_ptrWorker->requestStart();
}
void EmulatorController::stop()
{
    qDebug() << "EmulatorController: stop";
    m_ptrWorker->requestStop();
}
void EmulatorController::reset()
{
    qDebug() << "EmulatorController: reset";
    m_ptrWorker->requestReset();
}
void EmulatorController::setSpeed(Emulator::Speed speed)
{
    qDebug() << "EmulatorController: setSpeed";
    m_ptrWorker->requestSpeed(speed);
}
