#pragma once

#include <QObject>
#include <QByteArray>
#include <QSharedPointer>

#include "EmulatorTypes.h"
#include "EmulatorRequest.h"
#include "EmulatorResponse.h"

#include "C64/C64ROMSet.h"
#include "Input/InputEvent.h"
#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECResponse.h"
#include "C64/IEC/IECSystemEvent.h"
#include "C64/IEC/IECDeviceEvent.h"


class EmulatorThread;
class EmulatorWorker;


class EmulatorController : public QObject
{
    Q_OBJECT

public:
    explicit EmulatorController(QObject* parent = nullptr);
    virtual ~EmulatorController();

public slots:
    // Requests
    EmulatorRequest::RequestId request(EmulatorRequest* ptrRequest);
    IECRequest::RequestId request(IECRequest* ptrRequest);

    // Emulator operations
    void loadROMSet(const C64ROMSet& romSet);
    void loadProgram(quint16 address, const QByteArray& data);
    void input(const InputEvent& event);

    void powerOn();
    void powerOff();
    void pause(bool bPaused);
    void softReset();
    void hardReset();
    void setSpeed(Emulator::Speed speed);

signals:
    // Request responses
    void requestDone(QSharedPointer<EmulatorResponse> response);
    void iecRequestDone(QSharedPointer<IECResponse> response);

    // Emulator state
    void stateChanged(Emulator::State state);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

    // IEC events
    void iecSystemEventsChanged(IECSystemEventList events);
    void iecDeviceEventsChanged(IECDeviceEventList events);

private:
    EmulatorThread* m_ptrThread = nullptr;
    EmulatorWorker* m_ptrWorker = nullptr;

    quint64 m_nextRequestId = 1;
};
