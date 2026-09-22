#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

#include "Emulator/EmulatorSpeed.h"

class C64Machine;


class EmulatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit EmulatorWorker(QObject *parent = nullptr);
    virtual ~EmulatorWorker();

    void run();

    void requestStart();
    void requestStop();
    void requestReset();
    void requestSpeed(Emulator::Speed speed);
    void requestShutdown();

signals:
    void runningChanged(bool bRunning);
    void speedChanged(Emulator::Speed spped);
    void statisticsChanged(quint64 cycles, quint64 cyclesPerSecond);

private:
    bool waitForRequests();
    bool processRequests();
    quint64 targetCyclesPerSecond() const;
    void synchronizeSpeed();

private:
    C64Machine *m_ptrMachine = nullptr;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    // Current state
    bool m_bRunning = false;
    Emulator::Speed m_speed = Emulator::Speed::Normal;
    QElapsedTimer m_speedTimer;
    quint64 m_speedStartCycles = 0;

    // Request
    bool m_bStartRequested = false;
    bool m_bResetRequested = false;
    bool m_bStopRequested = false;
    bool m_bShutdownRequested = false;
    bool m_bSpeedRequested = false;
    Emulator::Speed m_speedRequested = Emulator::Speed::Normal;

    // Statistics
    quint64 m_totalCycles = 0;
    quint64 m_measuredCycles = 0;
};
