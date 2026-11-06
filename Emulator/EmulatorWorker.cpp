#include "EmulatorWorker.h"

#include <QDebug>
#include <QMutexLocker>
#include <QThread>
#include <QMetaType>

#include "EmulationScheduler.h"

#include "C64/C64Machine.h"
#include "C64/C64Timing.h"

#include "C64/IEC/IECDeviceFactory.h"
#include "C64/IEC/IECSystem.h"
#include "C64/IEC/IECRequest.h"
#include "C64/IEC/IECResponse.h"
#include "C64/IEC/IECSystemEvent.h"
#include "C64/IEC/IECDeviceEvent.h"


EmulatorWorker::EmulatorWorker(QObject* parent)
    : QObject{parent}
{
    qRegisterMetaType<QSharedPointer<EmulatorResponse>>("EmulatorResponsePtr");
    qRegisterMetaType<QSharedPointer<IECResponse>>("IECResponsePtr");

    qRegisterMetaType<QList<QSharedPointer<IECSystemEvent>>>("IECSystemEventList");
    qRegisterMetaType<IECDeviceEventList>("IECDeviceEventList");

    // Runtime-Komponenten in Abhängigkeitsreihenfolge erzeugen.
    m_ptrMachine = new C64Machine();
    m_ptrIECDeviceFactory = new IECDeviceFactory();
    m_ptrIECSystem = new IECSystem(m_ptrMachine, m_ptrIECDeviceFactory);
    m_ptrScheduler = new EmulationScheduler(m_ptrMachine, m_ptrIECSystem);

    qDebug() << "EmulatorWorker: created";
}


EmulatorWorker::~EmulatorWorker()
{
    // Der EmulatorThread muss vor dem Destruktor beendet worden sein.

    // Komponenten in umgekehrter Abhängigkeitsreihenfolge zerstören.
    delete m_ptrScheduler;
    m_ptrScheduler = nullptr;

    delete m_ptrIECSystem;
    m_ptrIECSystem = nullptr;

    delete m_ptrIECDeviceFactory;
    m_ptrIECDeviceFactory = nullptr;

    delete m_ptrMachine;
    m_ptrMachine = nullptr;

    qDebug() << "EmulatorWorker: destroyed";
}


void EmulatorWorker::run()
{
    qDebug() << "EmulatorWorker: run() started" << QThread::currentThreadId();

    const quint64 cyclesPerBatch = 500;
    QElapsedTimer statisticsTimer;

    while (!m_bShutdown)
    {
        // Bei ausgeschalteter oder pausierter Maschine auf Requests warten.
        if (m_state != Emulator::State::Running)
            waitForRequests();

        // Pro Schleifendurchlauf maximal einen Request ausführen.
        bool bRestartTiming = false;
        processNextRequest(bRestartTiming);

        if (m_bShutdown)
            break;

        if (bRestartTiming)
        {
            m_measuredCycles = 0;
            statisticsTimer.restart();
        }

        // Ein Scheduler-Batch pro Schleifendurchlauf.
        if (m_state != Emulator::State::Running)
            continue;

        m_ptrScheduler->runCycles(cyclesPerBatch);

        m_totalCycles += cyclesPerBatch;
        m_measuredCycles += cyclesPerBatch;

        forwardIECEvents();

        synchronizeSpeed();

        if (statisticsTimer.elapsed() >= 100)
        {
            const quint64 elapsed = static_cast<quint64>(statisticsTimer.elapsed());

            const quint64 currentCyclesPerSecond = (m_measuredCycles * 1000) / elapsed;

            const quint64 speedElapsedNanoseconds = static_cast<quint64>(m_speedTimer.nsecsElapsed());
            const quint64 speedElapsedCycles = m_totalCycles - m_speedStartCycles;

            quint64 averageCyclesPerSecond = 0;

            if (speedElapsedNanoseconds > 0)
                averageCyclesPerSecond = (speedElapsedCycles * 1000000000ULL) / speedElapsedNanoseconds;

            emit statisticsChanged(m_totalCycles, currentCyclesPerSecond, averageCyclesPerSecond);

            m_measuredCycles = 0;
            statisticsTimer.restart();
        }
    }

    // Der Worker beendet die Maschine genau einmal.
    m_ptrMachine->powerOff();

    if (m_state != Emulator::State::PoweredOff)
    {
        m_state = Emulator::State::PoweredOff;
        emit stateChanged(m_state);
    }

    qDebug() << "EmulatorWorker: run() finished";
}


void EmulatorWorker::request(EmulatorRequest* ptrRequest)
{
    if (ptrRequest == nullptr)
        return;

    bool bAccepted = false;

    {
        QMutexLocker locker(&m_mutex);

        if (m_bAcceptRequests)
        {
            PendingRequest request;
            request.type = PendingRequest::Type::Emulator;
            request.ptrEmulatorRequest = ptrRequest;

            m_requests.enqueue(request);
            bAccepted = true;

            // Nach Shutdown keine weiteren Requests mehr annehmen.
            // Bereits eingereihte Requests bleiben in FIFO-Reihenfolge erhalten.
            if (ptrRequest->type() == EmulatorRequest::Type::Shutdown)
                m_bAcceptRequests = false;

            m_waitCondition.wakeOne();
        }
    }

    if (!bAccepted)
    {
        const QSharedPointer<EmulatorResponse> response(
            new EmulatorResponse(ptrRequest->id(), ptrRequest->type(), EmulatorResponse::Result::Cancelled));

        delete ptrRequest;
        emit requestDone(response);
    }
}


void EmulatorWorker::request(IECRequest* ptrRequest)
{
    if (ptrRequest == nullptr)
        return;

    bool bAccepted = false;

    {
        QMutexLocker locker(&m_mutex);

        if (m_bAcceptRequests)
        {
            PendingRequest request;
            request.type = PendingRequest::Type::IEC;
            request.ptrIECRequest = ptrRequest;

            m_requests.enqueue(request);
            bAccepted = true;

            m_waitCondition.wakeOne();
        }
    }

    if (!bAccepted)
    {
        QSharedPointer<IECResponse> response(ptrRequest->createResponse());

        if (!response.isNull())
        {
            response->setResult(IECResponse::Result::Cancelled);
            emit iecRequestDone(response);
        }

        delete ptrRequest;
    }
}


void EmulatorWorker::waitForRequests()
{
    QMutexLocker locker(&m_mutex);

    while (m_requests.isEmpty())
        m_waitCondition.wait(&m_mutex);
}


void EmulatorWorker::processNextRequest(bool& bRestartTiming)
{
    PendingRequest request;

    {
        QMutexLocker locker(&m_mutex);

        if (m_requests.isEmpty())
            return;

        request = m_requests.dequeue();
    }

    if (request.type == PendingRequest::Type::IEC)
    {
        IECRequest* ptrIECRequest = request.ptrIECRequest;

        if (ptrIECRequest == nullptr)
            return;

        QSharedPointer<IECResponse> response(ptrIECRequest->createResponse());

        if (response.isNull())
        {
            delete ptrIECRequest;
            return;
        }

        const IECResponse::Result result = processIECRequest(ptrIECRequest, *response);
        response->setResult(result);

        delete ptrIECRequest;

        forwardIECEvents();

        emit iecRequestDone(response);

        return;
    }

    EmulatorRequest* ptrEmulatorRequest = request.ptrEmulatorRequest;

    if (ptrEmulatorRequest == nullptr)
        return;

    const EmulatorResponse::Result result = processRequest(ptrEmulatorRequest, bRestartTiming);

    const QSharedPointer<EmulatorResponse> response(
        new EmulatorResponse(ptrEmulatorRequest->id(), ptrEmulatorRequest->type(), result));

    delete ptrEmulatorRequest;

    if (bRestartTiming && !m_bShutdown)
    {
        m_speedStartCycles = m_totalCycles;
        m_speedTimer.restart();
        m_measuredCycles = 0;
    }

    forwardIECEvents();

    emit requestDone(response);
}


EmulatorResponse::Result EmulatorWorker::processRequest(const EmulatorRequest* ptrRequest, bool& bRestartTiming)
{
    switch (ptrRequest->type())
    {
    case EmulatorRequest::Type::LoadROMSet:
    {
        if (m_state != Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        const EmulatorRequestLoadROMSet* ptrROMRequest =
            static_cast<const EmulatorRequestLoadROMSet*>(ptrRequest);

        if (!m_ptrMachine->loadROMSet(ptrROMRequest->romSet()))
        {
            qWarning() << "EmulatorWorker: ROM set could not be loaded";
            return EmulatorResponse::Result::Failed;
        }

        qDebug() << "EmulatorWorker: ROM set loaded";

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::LoadProgram:
    {
        if (m_state != Emulator::State::Paused)
            return EmulatorResponse::Result::InvalidState;

        const EmulatorRequestLoadProgram* ptrProgramRequest =
            static_cast<const EmulatorRequestLoadProgram*>(ptrRequest);

        if (!m_ptrMachine->loadProgram(ptrProgramRequest->address(), ptrProgramRequest->data()))
        {
            qWarning() << "EmulatorWorker: program could not be loaded";
            return EmulatorResponse::Result::Failed;
        }

        qDebug() << "EmulatorWorker: program loaded";

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::Input:
    {
        if (m_state == Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        const EmulatorRequestInput* ptrInputRequest =
            static_cast<const EmulatorRequestInput*>(ptrRequest);

        const InputEvent& event = ptrInputRequest->event();

        switch (event.type)
        {
        case InputEventType::KeyPress:
            m_ptrMachine->keyPress(event.key);
            break;

        case InputEventType::KeyRelease:
            m_ptrMachine->keyRelease(event.key);
            break;

        default:
            return EmulatorResponse::Result::InvalidParameter;
        }

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::PowerOn:
    {
        if (m_state != Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        m_ptrMachine->powerOn();
        m_ptrIECSystem->reset();

        m_totalCycles = 0;
        m_measuredCycles = 0;

        m_state = Emulator::State::Running;
        bRestartTiming = true;

        emit stateChanged(m_state);
        emit statisticsChanged(m_totalCycles, 0, 0);

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::PowerOff:
    {
        if (m_state == Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        m_ptrMachine->powerOff();

        m_state = Emulator::State::PoweredOff;

        emit stateChanged(m_state);
        emit statisticsChanged(m_totalCycles, 0, 0);

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::Pause:
    {
        const EmulatorRequestPause* ptrPauseRequest =
            static_cast<const EmulatorRequestPause*>(ptrRequest);

        if (ptrPauseRequest->paused())
        {
            if (m_state != Emulator::State::Running)
                return EmulatorResponse::Result::InvalidState;

            m_state = Emulator::State::Paused;

            emit stateChanged(m_state);
            emit statisticsChanged(m_totalCycles, 0, 0);
        }
        else
        {
            if (m_state != Emulator::State::Paused)
                return EmulatorResponse::Result::InvalidState;

            m_state = Emulator::State::Running;
            bRestartTiming = true;

            emit stateChanged(m_state);
        }

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::SoftReset:
    {
        if (m_state == Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        m_ptrMachine->softReset();

        if (m_state == Emulator::State::Running)
            bRestartTiming = true;

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::HardReset:
    {
        if (m_state == Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        m_ptrMachine->hardReset();
        m_ptrIECSystem->reset();

        if (m_state == Emulator::State::Running)
            bRestartTiming = true;

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::SetSpeed:
    {
        const EmulatorRequestSetSpeed* ptrSpeedRequest =
            static_cast<const EmulatorRequestSetSpeed*>(ptrRequest);

        switch (ptrSpeedRequest->speed())
        {
        case Emulator::Speed::Half:
        case Emulator::Speed::Normal:
        case Emulator::Speed::Double:
        case Emulator::Speed::Five:
        case Emulator::Speed::Warp:
            break;

        default:
            return EmulatorResponse::Result::InvalidParameter;
        }

        if (m_speed != ptrSpeedRequest->speed())
        {
            m_speed = ptrSpeedRequest->speed();

            if (m_state == Emulator::State::Running)
                bRestartTiming = true;

            emit speedChanged(m_speed);
        }

        return EmulatorResponse::Result::Success;
    }

    case EmulatorRequest::Type::Shutdown:
    {
        m_bShutdown = true;

        return EmulatorResponse::Result::Success;
    }

    default:
        return EmulatorResponse::Result::InvalidParameter;
    }
}


IECResponse::Result EmulatorWorker::processIECRequest(const IECRequest* ptrRequest, IECResponse& response)
{
    return m_ptrIECSystem->processRequest(*ptrRequest, response);
}


quint64 EmulatorWorker::targetCyclesPerSecond() const
{
    const quint64 c64CyclesPerSecond = m_ptrMachine->timing().cyclesPerSecond;

    switch (m_speed)
    {
    case Emulator::Speed::Half:
        return c64CyclesPerSecond / 2;

    case Emulator::Speed::Normal:
        return c64CyclesPerSecond;

    case Emulator::Speed::Double:
        return c64CyclesPerSecond * 2;

    case Emulator::Speed::Five:
        return c64CyclesPerSecond * 5;

    case Emulator::Speed::Warp:
        return 0;
    }

    return c64CyclesPerSecond;
}


void EmulatorWorker::synchronizeSpeed()
{
    const quint64 targetCycles = targetCyclesPerSecond();

    if (targetCycles == 0)
        return;

    const quint64 elapsedNanoseconds = static_cast<quint64>(m_speedTimer.nsecsElapsed());
    const quint64 elapsedCycles = m_totalCycles - m_speedStartCycles;
    const quint64 targetNanoseconds = (elapsedCycles * 1000000000ULL) / targetCycles;

    if (targetNanoseconds <= elapsedNanoseconds)
        return;

    const quint64 waitNanoseconds = targetNanoseconds - elapsedNanoseconds;
    const quint64 waitMicroseconds = waitNanoseconds / 1000;

    if (waitMicroseconds > 0)
        QThread::usleep(static_cast<unsigned long>(waitMicroseconds));
}


void EmulatorWorker::forwardIECEvents()
{
    const QList<QSharedPointer<IECSystemEvent>> systemEvents =
        m_ptrIECSystem->takeSystemEvents();

    if (!systemEvents.isEmpty())
        emit iecSystemEventsChanged(systemEvents);

    const IECDeviceEventList deviceEvents =
        m_ptrIECSystem->takeDeviceEvents();

    if (!deviceEvents.isEmpty())
        emit iecDeviceEventsChanged(deviceEvents);
}
