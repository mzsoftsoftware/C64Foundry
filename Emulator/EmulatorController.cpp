#include "EmulatorController.h"

#include <QDebug>
#include <QThread>
#include <QMetaType>

#include "Emulator/EmulatorThread.h"
#include "Emulator/EmulatorWorker.h"


EmulatorController::EmulatorController(QObject* parent)
    : QObject(parent)
{
    qDebug() << "EmulatorController: created" << QThread::currentThreadId();

    qRegisterMetaType<QSharedPointer<EmulatorResponse>>("EmulatorResponsePtr");
    qRegisterMetaType<QSharedPointer<IECResponse>>("IECResponsePtr");

    qRegisterMetaType<IECSystemEventList>("IECSystemEventList");
    qRegisterMetaType<IECDeviceEventList>("IECDeviceEventList");

    m_ptrWorker = new EmulatorWorker();
    m_ptrThread = new EmulatorThread(m_ptrWorker, this);

    // Request responses
    connect(m_ptrWorker, &EmulatorWorker::requestDone,
            this, &EmulatorController::onWorkerRequestDone,
            Qt::QueuedConnection);

    connect(m_ptrWorker, &EmulatorWorker::iecRequestDone,
            this, &EmulatorController::iecRequestDone,
            Qt::QueuedConnection);

    // Emulator state
    connect(m_ptrWorker, &EmulatorWorker::stateChanged,
            this, &EmulatorController::onWorkerStateChanged,
            Qt::QueuedConnection);

    connect(m_ptrWorker, &EmulatorWorker::speedChanged,
            this, &EmulatorController::speedChanged,
            Qt::QueuedConnection);

    connect(m_ptrWorker, &EmulatorWorker::statisticsChanged,
            this, &EmulatorController::statisticsChanged,
            Qt::QueuedConnection);

    // IEC events
    connect(m_ptrWorker, &EmulatorWorker::iecSystemEventsChanged,
            this, &EmulatorController::iecSystemEventsChanged,
            Qt::QueuedConnection);

    connect(m_ptrWorker, &EmulatorWorker::iecDeviceEventsChanged,
            this, &EmulatorController::iecDeviceEventsChanged,
            Qt::QueuedConnection);

    m_ptrThread->start();
}


EmulatorController::~EmulatorController()
{
    qDebug() << "EmulatorController: shutting down";

    request(new EmulatorRequestShutdown());
    m_ptrThread->wait();

    delete m_ptrWorker;
    m_ptrWorker = nullptr;

    qDebug() << "EmulatorController: destroyed";
}


EmulatorRequest::RequestId EmulatorController::request(EmulatorRequest* ptrRequest)
{
    if (ptrRequest == nullptr)
        return 0;

    const EmulatorRequest::RequestId id = m_nextRequestId++;

    ptrRequest->setId(id);
    m_ptrWorker->request(ptrRequest);

    return id;
}


IECRequest::RequestId EmulatorController::request(IECRequest* ptrRequest)
{
    if (ptrRequest == nullptr)
        return 0;

    const IECRequest::RequestId id = m_nextRequestId++;

    ptrRequest->setId(id);
    m_ptrWorker->request(ptrRequest);

    return id;
}

void EmulatorController::loadROMSet(const C64ROMSet& romSet)
{
    request(new EmulatorRequestLoadROMSet(romSet));
}


void EmulatorController::input(const InputEvent& event)
{
    request(new EmulatorRequestInput(event));
}


void EmulatorController::powerOn()
{
    request(new EmulatorRequestPowerOn());
}


void EmulatorController::powerOff()
{
    cancelProgramLoad();
    request(new EmulatorRequestPowerOff());
}


void EmulatorController::pause(bool bPaused)
{
    request(new EmulatorRequestPause(bPaused));
}


void EmulatorController::softReset()
{
    request(new EmulatorRequestSoftReset());
}


void EmulatorController::hardReset()
{
    request(new EmulatorRequestHardReset());
}


void EmulatorController::setSpeed(Emulator::Speed speed)
{
    request(new EmulatorRequestSetSpeed(speed));
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
        request(new EmulatorRequestPowerOn());
        m_programRequestId = m_nextRequestId - 1;
        break;

    case Emulator::State::Running:
        m_programLoadState = ProgramLoadState::WaitingForPause;
        request(new EmulatorRequestPause(true));
        m_programRequestId = m_nextRequestId - 1;
        break;

    case Emulator::State::Paused:
        m_programLoadState = ProgramLoadState::WaitingForProgram;
        request(new EmulatorRequestLoadProgram(m_programAddress, m_programData));
        m_programRequestId = m_nextRequestId - 1;
        break;
    }
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

    emit requestDone(response);

    processProgramLoadResponse(response);
}


void EmulatorController::processProgramLoadResponse(const QSharedPointer<EmulatorResponse>& response)
{
    if (m_programLoadState == ProgramLoadState::Idle)
        return;

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
        request(new EmulatorRequestPause(true));
        m_programRequestId = m_nextRequestId - 1;
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
        request(new EmulatorRequestLoadProgram(m_programAddress, m_programData));
        m_programRequestId = m_nextRequestId - 1;
        break;
    }

    case ProgramLoadState::WaitingForProgram:
    {
        const bool bLoaded = bSuccess;

        if (m_bResumeAfterProgramLoad)
        {
            m_programLoadState = ProgramLoadState::WaitingForResume;
            request(new EmulatorRequestPause(false));
            m_programRequestId = m_nextRequestId - 1;
        }
        else
        {
            cancelProgramLoad();
        }

        emit programLoaded(bLoaded);
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
