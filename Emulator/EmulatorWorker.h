#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QQueue>
#include <QByteArray>

#include "EmulatorTypes.h"
#include "EmulationScheduler.h"
#include "Input/InputEvent.h"

class C64Machine;
class IECBus;
class IECDeviceManager;
#include "C64/C64ROMSet.h"


class EmulatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit EmulatorWorker(IECBus* ptrIECBus, QObject* parent = nullptr);
    virtual ~EmulatorWorker();

    // Getter
    C64Machine* machine() const                             { return m_ptrMachine; }

    // Operations
    void run();

    void requestROMSet(const C64ROMSet& romSet);
    void requestInput(const InputEvent& event);
    void requestProgram(quint16 address, const QByteArray& data);

    void requestPowerOn();
    void requestPowerOff();
    void requestPause(bool bPaused);
    void requestSoftReset();
    void requestHardReset();
    void requestSpeed(Emulator::Speed speed);
    void requestShutdown();

signals:
    void romSetLoaded(bool bLoaded);
    void programLoaded(bool bLoaded);

    void stateChanged(Emulator::State state);
    void speedChanged(Emulator::Speed spped);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

private:
    bool waitForRequests();
    bool processRequests();
    quint64 targetCyclesPerSecond() const;
    void synchronizeSpeed();

    void registerIECDevices();
    void unregisterIECDevices();

private:
    C64Machine* m_ptrMachine = nullptr;
    EmulationScheduler* m_ptrScheduler = nullptr;
    IECDeviceManager* m_ptrIECDeviceManager = nullptr;

    QMutex m_mutex;
    QWaitCondition m_waitCondition;

    // Current state
    Emulator::State m_state = Emulator::State::PoweredOff;
    Emulator::Speed m_speed = Emulator::Speed::Normal;
    QElapsedTimer m_speedTimer;
    quint64 m_speedStartCycles = 0;

    // Request
    bool m_bROMSetRequested = false;
    C64ROMSet m_romSetRequested;

    QQueue<InputEvent> m_inputEvents;

    bool m_bProgramRequested = false;
    quint16 m_programAddressRequested = 0;
    QByteArray m_programDataRequested;

    bool m_bPowerOnRequested = false;
    bool m_bPowerOffRequested = false;

    bool m_bPauseRequested = false;
    bool m_bPauseRequestedValue = false;

    bool m_bSoftResetRequested = false;
    bool m_bHardResetRequested = false;

    bool m_bSpeedRequested = false;
    Emulator::Speed m_speedRequested = Emulator::Speed::Normal;

    bool m_bShutdownRequested = false;

    // Statistics
    quint64 m_totalCycles = 0;
    quint64 m_measuredCycles = 0;
};
