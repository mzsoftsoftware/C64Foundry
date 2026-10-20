#include "EmulatorController.h"

#include <QDebug>
#include <QThread>
#include <QMetaType>

#include "Emulator/EmulatorThread.h"
#include "Emulator/EmulatorWorker.h"

#include "C64/IEC/IECBus.h"


EmulatorController::EmulatorController(QObject* parent)
    : QObject(parent)
{
    qDebug() << "EmulatorController: created" << QThread::currentThreadId();

    qRegisterMetaType<QSharedPointer<EmulatorResponse>>("EmulatorResponsePtr");

    m_ptrIECBus = new IECBus();
    m_ptrWorker = new EmulatorWorker(m_ptrIECBus);
    m_ptrThread = new EmulatorThread(m_ptrWorker, this);

    connect(m_ptrWorker, &EmulatorWorker::requestDone, this, &EmulatorController::onWorkerRequestDone);
    connect(m_ptrWorker, &EmulatorWorker::stateChanged, this, &EmulatorController::onWorkerStateChanged);
    connect(m_ptrWorker, &EmulatorWorker::speedChanged, this, &EmulatorController::speedChanged);
    connect(m_ptrWorker, &EmulatorWorker::statisticsChanged, this, &EmulatorController::statisticsChanged);

    m_ptrThread->start();
}

EmulatorController::~EmulatorController()
{
    qDebug() << "EmulatorController: shutting down";

    m_ptrWorker->request(new EmulatorRequestShutdown(nextRequestId()));
    m_ptrThread->wait();

    delete m_ptrWorker;
    delete m_ptrIECBus;

    qDebug() << "EmulatorController: destroyed";
}


C64Machine* EmulatorController::machine() const
{
    return m_ptrWorker->machine();
}


EmulatorRequest::RequestId EmulatorController::nextRequestId()
{
    return m_nextRequestId++;
}


void EmulatorController::request(EmulatorRequest* ptrRequest)
{
    if (ptrRequest == nullptr)
        return;

    m_ptrWorker->request(ptrRequest);
}


void EmulatorController::loadROMSet(const C64ROMSet& romSet)
{
    qDebug() << "EmulatorController: load ROM set";

    request(new EmulatorRequestLoadROMSet(nextRequestId(), romSet));
}


void EmulatorController::input(const InputEvent& event)
{
    request(new EmulatorRequestInput(nextRequestId(), event));
}


void EmulatorController::loadProgram(quint16 address, const QByteArray& data)
{
    if (m_programLoadState != ProgramLoadState::Idle)
    {
        qWarning() << "EmulatorController: program load already in progress";
        emit programLoaded(false);
        return;
    }

    m_programAddress = address;
    m_programData = data;

    m_bResumeAfterProgramLoad = m_state != Emulator::State::Paused;

    switch (m_state)
    {
    case Emulator::State::PoweredOff:
        m_programLoadState = ProgramLoadState::WaitingForPowerOn;
        m_programRequestId = nextRequestId();
        request(new EmulatorRequestPowerOn(m_programRequestId));
        break;

    case Emulator::State::Running:
        m_programLoadState = ProgramLoadState::WaitingForPause;
        m_programRequestId = nextRequestId();
        request(new EmulatorRequestPause(m_programRequestId, true));
        break;

    case Emulator::State::Paused:
        m_programLoadState = ProgramLoadState::WaitingForProgram;
        m_programRequestId = nextRequestId();
        request(new EmulatorRequestLoadProgram(m_programRequestId, m_programAddress, m_programData));
        break;
    }
}


void EmulatorController::powerOn()
{
    qDebug() << "EmulatorController: powerOn";

    request(new EmulatorRequestPowerOn(nextRequestId()));
}


void EmulatorController::powerOff()
{
    qDebug() << "EmulatorController: powerOff";

    cancelProgramLoad();
    request(new EmulatorRequestPowerOff(nextRequestId()));
}


void EmulatorController::pause(bool bPaused)
{
    qDebug() << "EmulatorController: pause" << bPaused;

    request(new EmulatorRequestPause(nextRequestId(), bPaused));
}


void EmulatorController::softReset()
{
    qDebug() << "EmulatorController: softReset";

    request(new EmulatorRequestSoftReset(nextRequestId()));
}


void EmulatorController::hardReset()
{
    qDebug() << "EmulatorController: hardReset";

    request(new EmulatorRequestHardReset(nextRequestId()));
}


void EmulatorController::setSpeed(Emulator::Speed speed)
{
    qDebug() << "EmulatorController: setSpeed" << speed;

    request(new EmulatorRequestSetSpeed(nextRequestId(), speed));
}


void EmulatorController::onWorkerStateChanged(Emulator::State state)
{
    m_state = state;

    emit stateChanged(state);
}


void EmulatorController::onWorkerRequestDone(QSharedPointer<EmulatorResponse> response)
{
    if (response.isNull())
        return;

    // Alle Responses werden zunächst an die GUI weitergegeben.
    emit requestDone(response);

    // Bestehende Signale für die GUI erhalten.
    switch (response->type())
    {
    case EmulatorRequest::Type::LoadROMSet:
        emit romSetLoaded(response->success());
        break;

    default:
        break;
    }

    // Eine Response kann gleichzeitig zu einem internen
    // Programmlade-Schritt gehören.
    processProgramLoadResponse(response);
}


void EmulatorController::processProgramLoadResponse(const QSharedPointer<EmulatorResponse>& response)
{
    if (m_programLoadState == ProgramLoadState::Idle)
        return;

    // Nur die Response des aktuell erwarteten Requests auswerten.
    if (response->id() != m_programRequestId)
        return;

    const bool bSuccess = response->success();

    switch (m_programLoadState)
    {
    case ProgramLoadState::WaitingForPowerOn:
    {
        if (!bSuccess)
        {
            emit programLoaded(false);
            cancelProgramLoad();
            return;
        }

        m_programLoadState = ProgramLoadState::WaitingForPause;
        m_programRequestId = nextRequestId();

        request(new EmulatorRequestPause(m_programRequestId, true));
        break;
    }

    case ProgramLoadState::WaitingForPause:
    {
        if (!bSuccess)
        {
            emit programLoaded(false);
            cancelProgramLoad();
            return;
        }

        m_programLoadState = ProgramLoadState::WaitingForProgram;
        m_programRequestId = nextRequestId();

        request(new EmulatorRequestLoadProgram(m_programRequestId, m_programAddress, m_programData));
        break;
    }

    case ProgramLoadState::WaitingForProgram:
    {
        emit programLoaded(bSuccess);

        if (m_bResumeAfterProgramLoad)
        {
            m_programLoadState = ProgramLoadState::WaitingForResume;
            m_programRequestId = nextRequestId();

            request(new EmulatorRequestPause(m_programRequestId, false));
        }
        else
        {
            cancelProgramLoad();
        }

        break;
    }

    case ProgramLoadState::WaitingForResume:
    {
        if (!bSuccess)
            qWarning() << "EmulatorController: resume after program load failed";

        cancelProgramLoad();
        break;
    }

    case ProgramLoadState::Idle:
        break;
    }
}


void EmulatorController::cancelProgramLoad()
{
    m_programLoadState = ProgramLoadState::Idle;
    m_programRequestId = 0;

    m_programAddress = 0;
    m_programData.clear();

    m_bResumeAfterProgramLoad = false;
}
