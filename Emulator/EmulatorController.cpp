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
            this, &EmulatorController::requestDone,
            Qt::QueuedConnection);

    connect(m_ptrWorker, &EmulatorWorker::iecRequestDone,
            this, &EmulatorController::iecRequestDone,
            Qt::QueuedConnection);

    // Emulator state
    connect(m_ptrWorker, &EmulatorWorker::stateChanged,
            this, &EmulatorController::stateChanged,
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


void EmulatorController::loadProgram(quint16 address, const QByteArray& data)
{
    request(new EmulatorRequestLoadProgram(address, data));
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
