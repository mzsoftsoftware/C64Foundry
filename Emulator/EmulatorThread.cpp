#include "EmulatorThread.h"

#include "Emulator/EmulatorWorker.h"


EmulatorThread::EmulatorThread(EmulatorWorker* ptrWorker, QObject *parent)
    : QThread{parent}
    , m_ptrWorker(ptrWorker)
{
}
EmulatorThread::~EmulatorThread()
{
    m_ptrWorker = nullptr;
}

void EmulatorThread::run()
{
    m_ptrWorker->run();
}