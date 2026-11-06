#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QQueue>
#include <QSharedPointer>
#include <QList>

#include "EmulatorTypes.h"
#include "EmulatorRequest.h"
#include "EmulatorResponse.h"

class C64Machine;
class IECSystem;
class IECDeviceFactory;
class EmulationScheduler;

class IECRequest;
class IECResponse;
class IECSystemEvent;
class IECDeviceEvent;


class EmulatorWorker : public QObject
{
    Q_OBJECT

public:
    explicit EmulatorWorker(QObject* parent = nullptr);
    virtual ~EmulatorWorker();

    // Getter
    C64Machine* machine() const { return m_ptrMachine; }

    // Operations
    void run();
    void request(EmulatorRequest* ptrRequest);
    void request(IECRequest* ptrRequest);

signals:
    // Emulator/IEC requests
    void requestDone(QSharedPointer<EmulatorResponse> response);
    void iecRequestDone(QSharedPointer<IECResponse> response);

    // Emulator state
    void stateChanged(Emulator::State state);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

    // IEC events
    void iecSystemEventsChanged(QList<QSharedPointer<IECSystemEvent>> events);
    void iecDeviceEventsChanged(QList<QSharedPointer<IECDeviceEvent>> events);

private:
    struct PendingRequest
    {
        enum class Type
        {
            Emulator,
            IEC
        };

        Type type = Type::Emulator;

        EmulatorRequest* ptrEmulatorRequest = nullptr;
        IECRequest* ptrIECRequest = nullptr;
    };
    void waitForRequests();
    bool processNextRequest();
    EmulatorResponse::Result processRequest(const EmulatorRequest* ptrRequest, bool& bRestartTiming);
    QSharedPointer<EmulatorResponse> processIECRequest(const EmulatorRequest* ptrRequest);
    void cancelPendingRequest(PendingRequest& request);

    quint64 targetCyclesPerSecond() const;
    void synchronizeSpeed();

    void forwardIECEvents();

private:
    // Core components
    C64Machine* m_ptrMachine = nullptr;
    IECDeviceFactory* m_ptrIECDeviceFactory = nullptr;
    IECSystem* m_ptrIECSystem = nullptr;
    EmulationScheduler* m_ptrScheduler = nullptr;

    // Request queue
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QQueue<PendingRequest> m_requests;
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
