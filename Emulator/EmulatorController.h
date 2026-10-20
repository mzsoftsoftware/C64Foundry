#pragma once

#include <QObject>
#include <QByteArray>

#include "EmulatorTypes.h"
#include "EmulatorRequest.h"
#include "EmulatorResponse.h"

#include "C64/C64ROMSet.h"
#include "Input/InputEvent.h"

class EmulatorThread;
class EmulatorWorker;
class C64Machine;
class IECBus;


class EmulatorController : public QObject
{
    Q_OBJECT

public:
    explicit EmulatorController(QObject* parent = nullptr);
    virtual ~EmulatorController();

    // Getter
    C64Machine* machine() const;

    // Requests
    EmulatorRequest::RequestId request(EmulatorRequest* ptrRequest);

public slots:
    void loadROMSet(const C64ROMSet& romSet);
    void input(const InputEvent& event);
    void loadProgram(quint16 address, const QByteArray& data);

    void powerOn();
    void powerOff();
    void pause(bool bPaused);
    void softReset();
    void hardReset();
    void setSpeed(Emulator::Speed speed);

signals:
    void requestDone(QSharedPointer<EmulatorResponse> response);

    // Bestehende GUI-Schnittstelle
    void romSetLoaded(bool bLoaded);
    void programLoaded(bool bLoaded);

    void stateChanged(Emulator::State state);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

private slots:
    void onWorkerRequestDone(QSharedPointer<EmulatorResponse> response);
    void onWorkerStateChanged(Emulator::State state);

private:
    enum class ProgramLoadState
    {
        Idle,
        WaitingForPowerOn,
        WaitingForPause,
        WaitingForProgram,
        WaitingForResume
    };

    void cancelProgramLoad();
    void processProgramLoadResponse(const QSharedPointer<EmulatorResponse>& response);

private:
    EmulatorThread* m_ptrThread = nullptr;
    EmulatorWorker* m_ptrWorker = nullptr;
    IECBus* m_ptrIECBus = nullptr;

    Emulator::State m_state = Emulator::State::PoweredOff;

    EmulatorRequest::RequestId m_nextRequestId = 1;

    // Programmladen
    ProgramLoadState m_programLoadState = ProgramLoadState::Idle;
    EmulatorRequest::RequestId m_programRequestId = 0;

    quint16 m_programAddress = 0;
    QByteArray m_programData;

    bool m_bResumeAfterProgramLoad = false;
};
