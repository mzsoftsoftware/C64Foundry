#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QQueue>

#include "EmulatorTypes.h"
#include "EmulationScheduler.h"
#include "EmulatorRequest.h"
#include "EmulatorResponse.h"

class C64Machine;
class IECBus;
class IECDeviceManager;


class EmulatorWorker : public QObject
{
    Q_OBJECT
public:
    explicit EmulatorWorker(IECBus* ptrIECBus, QObject* parent = nullptr);
    virtual ~EmulatorWorker();

    // Getter
    C64Machine* machine() const { return m_ptrMachine; }

    // Operations
    void run();
    void request(EmulatorRequest* ptrRequest);

signals:
    void requestDone(QSharedPointer<EmulatorResponse> response);

    void stateChanged(Emulator::State state);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

private:
    void waitForRequests();
    bool processNextRequest();
    EmulatorResponse::Result processRequest(const EmulatorRequest* ptrRequest, bool& bRestartTiming);

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
    QQueue<EmulatorRequest*> m_requests;

    bool m_bAcceptRequests = true;
    bool m_bShutdown = false;

    // Current state
    Emulator::State m_state = Emulator::State::PoweredOff;
    Emulator::Speed m_speed = Emulator::Speed::Normal;
    QElapsedTimer m_speedTimer;
    quint64 m_speedStartCycles = 0;

    // Statistics
    quint64 m_totalCycles = 0;
    quint64 m_measuredCycles = 0;
};
