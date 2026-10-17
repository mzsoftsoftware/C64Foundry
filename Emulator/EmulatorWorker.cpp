#include "EmulatorWorker.h"

#include <QDebug>
#include <QThread>

#include "C64/C64Machine.h"
#include "C64/C64Timing.h"


EmulatorWorker::EmulatorWorker(IECBus* ptrIECBus, QObject* parent)
    : QObject{parent}
{
    m_ptrMachine = new C64Machine();
    m_ptrMachine->setIECBus(ptrIECBus);

    qDebug() << "EmulatorWorker: created";
}
EmulatorWorker::~EmulatorWorker()
{
    m_ptrMachine->powerOff();
    delete m_ptrMachine;

    qDebug() << "EmulatorWorker: destroyed";
}

void EmulatorWorker::run()
{
    qDebug() << "EmulatorWorker: run() started" << QThread::currentThreadId();

    const quint64 cyclesPerBatch = 500;
    QElapsedTimer statisticsTimer;

    while (true)
    {
        if (m_state != Emulator::State::Running)
        {
            if (!waitForRequests())
                break;
        }

        if (processRequests())
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

        if (m_state != Emulator::State::Running)
            continue;

        for (quint64 index = 0; index < cyclesPerBatch; ++index)
            m_ptrMachine->clock();

        m_totalCycles += cyclesPerBatch;
        m_measuredCycles += cyclesPerBatch;

        synchronizeSpeed();

        if (statisticsTimer.elapsed() >= 100)
        {
            const quint64 elapsed = static_cast<quint64>(statisticsTimer.elapsed());

            const quint64 currentCyclesPerSecond = (m_measuredCycles * 1000) / elapsed;

            const quint64 speedElapsedNanoseconds = static_cast<quint64>(m_speedTimer.nsecsElapsed());
            const quint64 speedElapsedCycles = m_totalCycles - m_speedStartCycles;

            quint64 averageCyclesPerSecond = 0;

            if (speedElapsedNanoseconds > 0)
                averageCyclesPerSecond = (speedElapsedCycles * 1000000000ULL) / speedElapsedNanoseconds;

            emit statisticsChanged(m_totalCycles, currentCyclesPerSecond, averageCyclesPerSecond);

            m_measuredCycles = 0;
            statisticsTimer.restart();
        }
    }

    m_ptrMachine->powerOff();

    qDebug() << "EmulatorWorker: run() finished";
}

void EmulatorWorker::requestROMSet(const C64ROMSet& romSet)
{
    QMutexLocker locker(&m_mutex);
    m_romSetRequested = romSet;
    m_bROMSetRequested = true;
    qDebug() << "EmulatorWorker: ROM set requested";
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestInput(const InputEvent& event)
{
    QMutexLocker locker(&m_mutex);
    m_inputEvents.enqueue(event);
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestProgram(const quint16 address, const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    m_programAddressRequested = address;
    m_programDataRequested = data;
    m_bProgramRequested = true;
    qDebug() << "EmulatorWorker: program requested";
    m_waitCondition.wakeOne();
}

void EmulatorWorker::requestPowerOn()
{
    QMutexLocker locker(&m_mutex);
    m_bPowerOnRequested = true;
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestPowerOff()
{
    QMutexLocker locker(&m_mutex);
    m_bPowerOffRequested = true;
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestPause(bool bPaused)
{
    QMutexLocker locker(&m_mutex);
    m_bPauseRequested = true;
    m_bPauseRequestedValue = bPaused;
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestSoftReset()
{
    QMutexLocker locker(&m_mutex);
    m_bSoftResetRequested = true;
    m_waitCondition.wakeOne();
}
void EmulatorWorker::requestHardReset()
{
    QMutexLocker locker(&m_mutex);
    m_bHardResetRequested = true;
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
void EmulatorWorker::requestShutdown()
{
    QMutexLocker locker(&m_mutex);
    m_bShutdownRequested = true;
    qDebug() << "EmulatorWorker: shutdown requested";
    m_waitCondition.wakeOne();
}

bool EmulatorWorker::waitForRequests()
{
    QMutexLocker locker(&m_mutex);
    while (!m_bROMSetRequested && m_inputEvents.isEmpty() && !m_bProgramRequested &&
           !m_bPowerOnRequested && !m_bPowerOffRequested && !m_bPauseRequested &&
           !m_bSoftResetRequested && !m_bHardResetRequested &&
           !m_bSpeedRequested && !m_bShutdownRequested)
    {
        m_waitCondition.wait(&m_mutex);
    }
    return !m_bShutdownRequested;
}

bool EmulatorWorker::processRequests()
{
    bool bROMSetRequested = false;
    C64ROMSet romSetRequested;

    QQueue<InputEvent> inputEvents;

    bool bProgramRequested = false;
    quint16 programAddressRequested = 0;
    QByteArray programDataRequested;

    bool bPowerOnRequested = false;
    bool bPowerOffRequested = false;

    bool bPauseRequested = false;
    bool bPauseRequestedValue = false;

    bool bSoftResetRequested = false;
    bool bHardResetRequested = false;

    bool bSpeedRequested = false;    
    Emulator::Speed speedRequested = Emulator::Speed::Normal;

    {
        QMutexLocker locker(&m_mutex);

        bROMSetRequested = m_bROMSetRequested;
        romSetRequested = m_romSetRequested;
        inputEvents.swap(m_inputEvents);
        bProgramRequested = m_bProgramRequested;
        programAddressRequested = m_programAddressRequested;
        programDataRequested.swap(m_programDataRequested);

        bPowerOnRequested = m_bPowerOnRequested;
        bPowerOffRequested = m_bPowerOffRequested;

        bPauseRequested = m_bPauseRequested;
        bPauseRequestedValue = m_bPauseRequestedValue;

        bSoftResetRequested = m_bSoftResetRequested;
        bHardResetRequested = m_bHardResetRequested;

        bSpeedRequested = m_bSpeedRequested;
        speedRequested = m_speedRequested;

        m_bROMSetRequested = false;
        m_bProgramRequested = false;

        m_bPowerOnRequested = false;
        m_bPowerOffRequested = false;
        m_bSoftResetRequested = false;
        m_bHardResetRequested = false;
        m_bSpeedRequested = false;
    }

    bool bRestartTiming = false;

    if (m_state == Emulator::State::PoweredOff)
    {
        if (bROMSetRequested)
        {
            bool bLoaded = false;
            bLoaded = m_ptrMachine->loadROMSet(romSetRequested);
            if (bLoaded)
                qDebug() << "EmulatorWorker: ROM set loaded";
            else
                qWarning() << "EmulatorWorker: ROM set could not be loaded";
            emit romSetLoaded(bLoaded);
        }
    }

    // Eingaben nur bei eingeschalteter Maschine.
    if (m_state != Emulator::State::PoweredOff)
    {
        while (!inputEvents.isEmpty())
        {
            const InputEvent event = inputEvents.dequeue();

            switch (event.type)
            {
            case InputEventType::KeyPress:
                m_ptrMachine->keyPress(event.key);
                break;

            case InputEventType::KeyRelease:
                m_ptrMachine->keyRelease(event.key);
                break;
            }
        }
    }

    if (bProgramRequested)
    {
        const bool bLoaded = m_ptrMachine->loadProgram(programAddressRequested, programDataRequested);
        if (bLoaded)
        {
            const quint32 endAddress = static_cast<quint32>(programAddressRequested) + static_cast<quint32>(programDataRequested.size());
            m_ptrMachine->setBasicProgramEnd(static_cast<quint16>(endAddress & 0xFFFF));
            qDebug() << "EmulatorWorker: program loaded"
                     << "start:" << Qt::hex << programAddressRequested
                     << "end:" << endAddress;
        }
        else
        {
            qWarning() << "EmulatorWorker: program could not be loaded";
        }
        emit programLoaded(bLoaded);
    }

    // PowerOff hat Vorrang vor PowerOn.
    if (bPowerOffRequested)
    {
        if (m_state != Emulator::State::PoweredOff)
        {
            m_ptrMachine->powerOff();
            m_state = Emulator::State::PoweredOff;

            emit stateChanged(m_state);
            emit statisticsChanged(m_totalCycles, 0, 0);
        }
    }
    else if (bPowerOnRequested)
    {
        if (m_state == Emulator::State::PoweredOff)
        {
            m_ptrMachine->powerOn();

            m_totalCycles = 0;
            m_measuredCycles = 0;

            m_state = Emulator::State::Running;
            bRestartTiming = true;

            emit stateChanged(m_state);
            emit statisticsChanged(m_totalCycles, 0, 0);
        }
    }
    // Pause und Resume.
    if (bPauseRequested)
    {
        if (bPauseRequestedValue && m_state == Emulator::State::Running)
        {
            m_state = Emulator::State::Paused;

            emit stateChanged(m_state);
            emit statisticsChanged(m_totalCycles, 0, 0);
        }
        else if (!bPauseRequestedValue && m_state == Emulator::State::Paused)
        {
            m_state = Emulator::State::Running;
            bRestartTiming = true;

            emit stateChanged(m_state);
        }
    }
    // Reset verändert den Betriebszustand nicht.
    if (m_state != Emulator::State::PoweredOff)
    {
        if (bHardResetRequested)
        {
            m_ptrMachine->hardReset();
            bRestartTiming = m_state == Emulator::State::Running;
        }
        else if (bSoftResetRequested)
        {
            m_ptrMachine->softReset();
            bRestartTiming = m_state == Emulator::State::Running;
        }
    }

    // Geschwindigkeit.
    if (bSpeedRequested && m_speed != speedRequested)
    {
        m_speed = speedRequested;

        if (m_state == Emulator::State::Running)
            bRestartTiming = true;

        emit speedChanged(m_speed);
    }

    if (bRestartTiming)
    {
        m_speedStartCycles = m_totalCycles;
        m_speedTimer.restart();
        m_measuredCycles = 0;
    }

    return bRestartTiming;
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
