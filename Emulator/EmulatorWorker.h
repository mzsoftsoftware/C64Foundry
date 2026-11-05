#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QQueue>
#include <QByteArray>

#include "EmulatorTypes.h"
#include "EmulatorRequest.h"
#include "EmulatorResponse.h"
#include "EmulatorIECDeviceEvent.h"

#include "C64/IEC/IECDeviceEvent.h"
class C64Machine;
class IECSystem;
class IECDeviceFactory;
class EmulationScheduler;


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

signals:
    void requestDone(QSharedPointer<EmulatorResponse> response);

    void stateChanged(Emulator::State state);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

    void iecDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void printerOutputChanged(IECDeviceConfiguration::DeviceId deviceId, QQueue<IECPrinterOutputEvent> events);
    void driveAccessChanged(IECDeviceConfiguration::DeviceId deviceId, QQueue<IECDriveAccessEvent> events);

private:
    void waitForRequests();
    bool processNextRequest();
    EmulatorResponse::Result processRequest(const EmulatorRequest* ptrRequest, bool& bRestartTiming);
    QSharedPointer<EmulatorResponse> processIECRequest(const EmulatorRequest* ptrRequest);

    quint64 targetCyclesPerSecond() const;
    void synchronizeSpeed();

    void publishIECDeviceEvent(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void forwardPrinterOutputChanges();
    void forwardDriveAccessChanges();

private:
    C64Machine* m_ptrMachine = nullptr;
    IECSystem* m_ptrIECSystem = nullptr;
    IECDeviceFactory* m_ptrIECDeviceFactory = nullptr;

    EmulationScheduler* m_ptrScheduler = nullptr;

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
