#include "EmulatorController.h"

#include <QDebug>

#include "Emulator/EmulatorThread.h"
#include "Emulator/EmulatorWorker.h"

#include "C64/IEC/IECBus.h"


EmulatorController::EmulatorController(QObject *parent)
    : QObject{parent}
{
    qDebug() << "EmulatorController: created" << QThread::currentThreadId();

    m_ptrIECBus = new IECBus();
    m_ptrWorker = new EmulatorWorker(m_ptrIECBus);
    m_ptrThread = new EmulatorThread(m_ptrWorker, this);

    connect(m_ptrWorker, &EmulatorWorker::romSetLoaded, this, &EmulatorController::romSetLoaded);
    connect(m_ptrWorker, &EmulatorWorker::programLoaded, this, &EmulatorController::onWorkerProgramLoaded);
    connect(m_ptrWorker, &EmulatorWorker::stateChanged, this, &EmulatorController::onWorkerStateChanged);    connect(m_ptrWorker, &EmulatorWorker::speedChanged, this, &EmulatorController::speedChanged);
    connect(m_ptrWorker, &EmulatorWorker::statisticsChanged, this, &EmulatorController::statisticsChanged);

    m_ptrThread->start();
}
EmulatorController::~EmulatorController()
{
    qDebug() << "EmulatorController: shutting down";

    m_ptrWorker->requestShutdown();
    m_ptrThread->wait();

    delete m_ptrWorker;
    delete m_ptrIECBus;

    qDebug() << "EmulatorController: destroyed";
}

C64Machine* EmulatorController::machine() const
{
    return m_ptrWorker->machine();
}

void EmulatorController::loadROMSet(const C64ROMSet& romSet)
{
    qDebug() << "EmulatorController: load ROM set";
    m_ptrWorker->requestROMSet(romSet);
}
void EmulatorController::loadProgram(const quint16 address, const QByteArray& data)
{
    if (m_programLoadState != ProgramLoadState::Idle)
    {
        qWarning() << "EmulatorController: program load already in progress";
        return;
    }

    m_programAddress = address;
    m_programData = data;

    m_bResumeAfterProgramLoad = m_state != Emulator::State::Paused;

    switch (m_state)
    {
    case Emulator::State::PoweredOff:
        m_programLoadState = ProgramLoadState::WaitingForPowerOn;
        m_ptrWorker->requestPowerOn();
        break;

    case Emulator::State::Running:
        m_programLoadState = ProgramLoadState::WaitingForPause;
        m_ptrWorker->requestPause(true);
        break;

    case Emulator::State::Paused:
        m_programLoadState = ProgramLoadState::WaitingForProgram;
        m_ptrWorker->requestProgram(m_programAddress, m_programData);
        break;
    }
}
void EmulatorController::input(const InputEvent& event)
{
    m_ptrWorker->requestInput(event);
}

void EmulatorController::powerOn()
{
    qDebug() << "EmulatorController: powerOn";
    m_ptrWorker->requestPowerOn();
}
void EmulatorController::powerOff()
{
    cancelProgramLoad();
    m_ptrWorker->requestPowerOff();
}
void EmulatorController::pause(bool bPaused)
{
    qDebug() << "EmulatorController: pause" << bPaused;
    m_ptrWorker->requestPause(bPaused);
}
void EmulatorController::softReset()
{
    qDebug() << "EmulatorController: softReset";
    m_ptrWorker->requestSoftReset();
}
void EmulatorController::hardReset()
{
    qDebug() << "EmulatorController: hardReset";
    m_ptrWorker->requestHardReset();
}
void EmulatorController::setSpeed(Emulator::Speed speed)
{
    qDebug() << "EmulatorController: setSpeed" << speed;
    m_ptrWorker->requestSpeed(speed);
}

void EmulatorController::onWorkerStateChanged(Emulator::State state)
{
    m_state = state;

    emit stateChanged(state);

    switch (m_programLoadState)
    {
    case ProgramLoadState::WaitingForPowerOn:
        if (state == Emulator::State::Running)
        {
            m_programLoadState = ProgramLoadState::WaitingForPause;
            m_ptrWorker->requestPause(true);
        }
        break;

    case ProgramLoadState::WaitingForPause:
        if (state == Emulator::State::Paused)
        {
            m_programLoadState = ProgramLoadState::WaitingForProgram;
            m_ptrWorker->requestProgram(m_programAddress, m_programData);
        }
        break;

    case ProgramLoadState::WaitingForResume:
        if (state == Emulator::State::Running)
            cancelProgramLoad();
        break;

    case ProgramLoadState::WaitingForProgram:
    case ProgramLoadState::Idle:
        break;
    }
}
void EmulatorController::onWorkerProgramLoaded(bool bLoaded)
{
    if (m_programLoadState != ProgramLoadState::WaitingForProgram)
        return;

    emit programLoaded(bLoaded);

    if (m_bResumeAfterProgramLoad && m_state == Emulator::State::Paused)
    {
        m_programLoadState = ProgramLoadState::WaitingForResume;
        m_ptrWorker->requestPause(false);
    }
    else
    {
        cancelProgramLoad();
    }
}
void EmulatorController::cancelProgramLoad()
{
    m_programLoadState = ProgramLoadState::Idle;
    m_programData.clear();
    m_bResumeAfterProgramLoad = false;
}
