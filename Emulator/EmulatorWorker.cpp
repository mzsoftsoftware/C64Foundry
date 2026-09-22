#include "EmulatorWorker.h"

#include <QDebug>
#include <QThread>

#include "C64/C64Machine.h"
#include "C64/C64Timing.h"


EmulatorWorker::EmulatorWorker(QObject *parent)
    : QObject{parent}
{
    m_ptrMachine = new C64Machine();

    qDebug() << "EmulatorWorker: created";
}
EmulatorWorker::~EmulatorWorker()
{
    delete m_ptrMachine;

    qDebug() << "EmulatorWorker: destroyed";
}

void EmulatorWorker::run()
{
    qDebug() << "EmulatorWorker: run() started" << QThread::currentThreadId();

    const quint64 cyclesPerBatch = 500; /// 1000 ist schlechter, 100 ist besser aber höhere CPU !
    QElapsedTimer statisticsTimer;

    while (true)
    {
        if (!m_bRunning)
        {
            if (!waitForRequests())
                break;
        }

        if(processRequests())
        {
            m_measuredCycles = 0;
            statisticsTimer.restart();
        }

        bool bShutdownRequested = false;
        {
            QMutexLocker locker(&m_mutex);
            bShutdownRequested = m_bShutdownRequested;
        }
        if (bShutdownRequested)
            break;

        if (!m_bRunning)
            continue;

        for (quint64 i = 0; i < cyclesPerBatch; ++i)
            m_ptrMachine->clock();
        m_totalCycles += cyclesPerBatch;
        m_measuredCycles += cyclesPerBatch;
        synchronizeSpeed();

        if (statisticsTimer.elapsed() >= 100)
        {
            const quint64 elapsed = statisticsTimer.elapsed();
            const quint64 cyclesPerSecond = (m_measuredCycles * 1000) / elapsed;
            emit statisticsChanged(m_totalCycles, cyclesPerSecond);
            m_measuredCycles = 0;
            statisticsTimer.restart();
        }
    }

    qDebug() << "EmulatorWorker: run() finished";
}

void EmulatorWorker::requestStart()
{
    QMutexLocker locker(&m_mutex);
    m_bStartRequested= true;
    qDebug() << "EmulatorWorker: start requested";
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestReset()
{
    QMutexLocker locker(&m_mutex);
    m_bResetRequested = true;
    qDebug() << "EmulatorWorker: reset requested";
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestStop()
{
    QMutexLocker locker(&m_mutex);
    m_bStopRequested = true;
    qDebug() << "EmulatorWorker: stop requested";
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestShutdown()
{
    QMutexLocker locker(&m_mutex);
    m_bShutdownRequested = true;
    qDebug() << "EmulatorWorker: shutdown requested";
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestSpeed(const Emulator::Speed speed)
{
    QMutexLocker locker(&m_mutex);
    m_bSpeedRequested = true;
    m_speedRequested = speed;
    qDebug() << "EmulatorWorker: speed requested";
    m_waitCondition.wakeOne();
}

bool EmulatorWorker::waitForRequests()
{
    QMutexLocker locker(&m_mutex);
    while (!m_bStartRequested && !m_bStopRequested &&
           !m_bResetRequested && !m_bShutdownRequested &&
           !m_bSpeedRequested)
        m_waitCondition.wait(&m_mutex);
    return !m_bShutdownRequested;
}

bool EmulatorWorker::processRequests()
{
    bool bStartRequested = false;
    bool bStopRequested = false;
    bool bResetRequested = false;
    bool bSpeedRequested = false;

    Emulator::Speed speedRequested = Emulator::Speed::Normal;

    {
        QMutexLocker locker(&m_mutex);

        bStartRequested = m_bStartRequested;
        bResetRequested = m_bResetRequested;
        bStopRequested = m_bStopRequested;
        bSpeedRequested = m_bSpeedRequested;
        speedRequested = m_speedRequested;

        m_bStartRequested = false;
        m_bResetRequested = false;
        m_bStopRequested = false;
        m_bSpeedRequested = false;
    }

    bool bRequested = false;

    if(bStartRequested)
    {
        if(!m_bRunning)
        {
            m_bRunning = true;
            bRequested = true;

            m_speedStartCycles = m_totalCycles;
            m_speedTimer.restart();

            emit runningChanged(m_bRunning);
        }
    }
    if (bResetRequested)
    {
        m_ptrMachine->reset();

        m_totalCycles = 0;
        m_measuredCycles = 0;

        m_speedStartCycles = 0;
        if (m_bRunning)
            m_speedTimer.restart();

        emit statisticsChanged(m_totalCycles, m_measuredCycles);
    }
    if(bStopRequested)
    {
        if(m_bRunning)
        {
            m_bRunning = false;
            emit runningChanged(m_bRunning);
        }
    }
    if (bSpeedRequested)
    {
        if (m_speed != speedRequested)
        {
            m_speed = speedRequested;

            m_speedStartCycles = m_totalCycles;
            m_speedTimer.restart();

            emit speedChanged(m_speed);
        }
    }

    return bRequested;
}

quint64 EmulatorWorker::targetCyclesPerSecond() const
{
    quint64 c64CyclesPerSecond = m_ptrMachine->timing().cyclesPerSecond;

    switch (m_speed)
    {
    case Emulator::Speed::Half:
        return c64CyclesPerSecond / 2;

    case Emulator::Speed::Normal:
        return c64CyclesPerSecond;

    case Emulator::Speed::Double:
        return c64CyclesPerSecond * 2;

    case Emulator::Speed::Five:
        return c64CyclesPerSecond * 5;

    case Emulator::Speed::Warp:
        return 0;
    }

    return c64CyclesPerSecond;
}

void EmulatorWorker::synchronizeSpeed()
{
    const quint64 targetCycles = targetCyclesPerSecond();
    if (targetCycles == 0)
        return;

    const quint64 elapsedNanoseconds = static_cast<quint64>(m_speedTimer.nsecsElapsed());
    const quint64 elapsedCycles = m_totalCycles - m_speedStartCycles;
    const quint64 targetNanoseconds = (elapsedCycles * 1000000000ULL) / targetCycles;
    if (targetNanoseconds <= elapsedNanoseconds)
        return;

    const quint64 waitNanoseconds = targetNanoseconds - elapsedNanoseconds;
    const quint64 waitMicroseconds = waitNanoseconds / 1000;
    if (waitMicroseconds > 0)
    {
        QThread::usleep(static_cast<unsigned long>(waitMicroseconds));
    }
}
