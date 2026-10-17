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
    connect(m_ptrWorker, &EmulatorWorker::programLoaded, this, &EmulatorController::programLoaded);
    connect(m_ptrWorker, &EmulatorWorker::stateChanged, this, &EmulatorController::stateChanged);
    connect(m_ptrWorker, &EmulatorWorker::speedChanged, this, &EmulatorController::speedChanged);
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
    qDebug() << "EmulatorController: load program";
    m_ptrWorker->requestProgram(address, data);
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
    qDebug() << "EmulatorController: powerOff";
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
