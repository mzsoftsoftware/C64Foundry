#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QQueue>
#include <QByteArray>
#include <QHash>

#include "EmulatorTypes.h"
#include "EmulationScheduler.h"
#include "EmulatorRequest.h"
#include "EmulatorResponse.h"
#include "EmulatorIECDeviceEvent.h"

class C64Machine;
class IECBus;
class IECDeviceManager;
class IECFoundryDrive;


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

    void iecDeviceChanged(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void printerOutputChanged(IECDeviceConfiguration::DeviceId deviceId, quint64 revision, QByteArray output);

private:
    void waitForRequests();
    bool processNextRequest();
    EmulatorResponse::Result processRequest(const EmulatorRequest* ptrRequest, bool& bRestartTiming);
    QSharedPointer<EmulatorResponse> processIECRequest(const EmulatorRequest* ptrRequest);

    quint64 targetCyclesPerSecond() const;
    void synchronizeSpeed();

    void registerIECDevices();
    void unregisterIECDevices();
    bool setIECDeviceConfiguration(const IECDeviceConfiguration& configuration, QString* operationMessage = nullptr);
    bool removeIECDevice(IECDeviceConfiguration::DeviceId id);

    void invalidatePrinterOutputRevision(IECDeviceConfiguration::DeviceId deviceId);
    void invalidateAllPrinterOutputRevisions();

    void publishIECDeviceEvent(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId);
    void publishPrinterOutputChanges();

private:
    C64Machine* m_ptrMachine = nullptr;
    EmulationScheduler* m_ptrScheduler = nullptr;
    IECDeviceManager* m_ptrIECDeviceManager = nullptr;
    IECFoundryDrive* m_ptrFoundryDrive = nullptr;

    QHash<IECDeviceConfiguration::DeviceId, quint64> m_printerOutputRevisions;

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
