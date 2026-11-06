#include "EmulatorWorker.h"

#include <QDebug>
#include <QMutexLocker>
#include <QThread>

#include "EmulationScheduler.h"

#include "C64/C64Machine.h"
#include "C64/C64Timing.h"
#include "C64/IEC/IECDeviceFactory.h"
#include "C64/IEC/IECSystem.h"


EmulatorWorker::EmulatorWorker(QObject* parent)
    : QObject{parent}
{
    qRegisterMetaType<EmulatorIECDeviceEvent::Type>("EmulatorIECDeviceEvent::Type");
    qRegisterMetaType<IECDriveAccessEvent>("IECDriveAccessEvent");
    qRegisterMetaType<QQueue<IECDriveAccessEvent>>("QQueue<IECDriveAccessEvent>");
    qRegisterMetaType<IECPrinterOutputEvent>("IECPrinterOutputEvent");
    qRegisterMetaType<QQueue<IECPrinterOutputEvent>>("QQueue<IECPrinterOutputEvent>");

    // Core components are created in dependency order.
    m_ptrMachine = new C64Machine();
    m_ptrIECDeviceFactory = new IECDeviceFactory();
    m_ptrIECSystem = new IECSystem(m_ptrMachine, m_ptrIECDeviceFactory);
    m_ptrScheduler = new EmulationScheduler(m_ptrMachine);

    qDebug() << "EmulatorWorker: created";
}

EmulatorWorker::~EmulatorWorker()
{
    // Der EmulatorThread muss vor dem Destruktor beendet worden sein.
    while (!m_requests.isEmpty())
        delete m_requests.dequeue();

    if (m_ptrMachine != nullptr)
        m_ptrMachine->powerOff();

    // Destroy components in reverse dependency order.
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
        // Die Response wird unmittelbar nach der Verarbeitung gesendet.
        const bool bRestartTiming = processNextRequest();

        if (m_bShutdown)
            break;

        if (bRestartTiming)
        {
            m_measuredCycles = 0;
            statisticsTimer.restart();
        }

        // Ein normaler Scheduler-Batch pro Schleifendurchlauf.
        if (m_state != Emulator::State::Running)
            continue;

        m_ptrScheduler->runCycles(cyclesPerBatch);
        forwardIECEvents();

        m_totalCycles += cyclesPerBatch;
        m_measuredCycles += cyclesPerBatch;

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

    m_ptrMachine->powerOff();

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
            m_requests.enqueue(ptrRequest);
            bAccepted = true;

            // Ab Shutdown keine weiteren Requests mehr annehmen.
            // Bereits eingereihte Requests bleiben in FIFO-Reihenfolge erhalten.
            if (ptrRequest->type() == EmulatorRequest::Type::Shutdown)
                m_bAcceptRequests = false;

            m_waitCondition.wakeOne();
        }
    }

    if (!bAccepted)
    {
        const QSharedPointer<EmulatorResponse> response(new EmulatorResponse(ptrRequest->id(), ptrRequest->type(), EmulatorResponse::Result::Cancelled));
        delete ptrRequest;
        emit requestDone(response);
    }
}


void EmulatorWorker::waitForRequests()
{
    QMutexLocker locker(&m_mutex);

    while (m_requests.isEmpty())
        m_waitCondition.wait(&m_mutex);
}



bool EmulatorWorker::processNextRequest()
{
    EmulatorRequest* ptrRequest = nullptr;
    {
        QMutexLocker locker(&m_mutex);

        if (m_requests.isEmpty())
            return false;

        ptrRequest = m_requests.dequeue();
    }

    bool bRestartTiming = false;
    QSharedPointer<EmulatorResponse> response;

    switch (ptrRequest->type())
    {
    case EmulatorRequest::Type::GetIECConfigurations:
    case EmulatorRequest::Type::AddIECDevice:
    case EmulatorRequest::Type::RemoveIECDevice:
    case EmulatorRequest::Type::SetIECConfiguration:
    case EmulatorRequest::Type::RestoreIECConfigurations:
    case EmulatorRequest::Type::TearOffIECPrinter:
        response = processIECRequest(ptrRequest);
        break;
    default:
    {
        const EmulatorResponse::Result result = processRequest(ptrRequest, bRestartTiming);
        response = QSharedPointer<EmulatorResponse>(new EmulatorResponse(ptrRequest->id(), ptrRequest->type(), result));
        break;
    }
    }

    if (bRestartTiming && !m_bShutdown)
    {
        m_speedStartCycles = m_totalCycles;
        m_speedTimer.restart();
        m_measuredCycles = 0;
    }

    delete ptrRequest;

    // Device operations may also have produced output or access events.
    forwardIECEvents();
    emit requestDone(response);

    return bRestartTiming;
}

EmulatorResponse::Result EmulatorWorker::processRequest(const EmulatorRequest* ptrRequest, bool& bRestartTiming)
{
    switch (ptrRequest->type())
    {
    case EmulatorRequest::Type::LoadROMSet:
    {
        if (m_state != Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        const EmulatorRequestLoadROMSet* ptrROMRequest = static_cast<const EmulatorRequestLoadROMSet*>(ptrRequest);

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

        const EmulatorRequestLoadProgram* ptrProgramRequest = static_cast<const EmulatorRequestLoadProgram*>(ptrRequest);

        if (!m_ptrMachine->loadProgram(ptrProgramRequest->address(), ptrProgramRequest->data()))
        {
            qWarning() << "EmulatorWorker: program could not be loaded";
            return EmulatorResponse::Result::Failed;
        }

        const quint32 endAddress = static_cast<quint32>(ptrProgramRequest->address()) + static_cast<quint32>(ptrProgramRequest->data().size());

        m_ptrMachine->setBasicProgramEnd(static_cast<quint16>(endAddress & 0xFFFF));

        qDebug() << "EmulatorWorker: program loaded";

        return EmulatorResponse::Result::Success;
    }
    case EmulatorRequest::Type::Input:
    {
        if (m_state == Emulator::State::PoweredOff)
            return EmulatorResponse::Result::InvalidState;

        const EmulatorRequestInput* ptrInputRequest = static_cast<const EmulatorRequestInput*>(ptrRequest);
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
        publishIECDeviceEvent(EmulatorIECDeviceEvent::Type::Reset, 0);

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
        const EmulatorRequestPause* ptrPauseRequest = static_cast<const EmulatorRequestPause*>(ptrRequest);

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
        publishIECDeviceEvent(EmulatorIECDeviceEvent::Type::Reset, 0);

        if (m_state == Emulator::State::Running)
            bRestartTiming = true;

        return EmulatorResponse::Result::Success;
    }
    case EmulatorRequest::Type::SetSpeed:
    {
        const EmulatorRequestSetSpeed* ptrSpeedRequest = static_cast<const EmulatorRequestSetSpeed*>(ptrRequest);

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
    case EmulatorRequest::Type::GetIECConfigurations:
    case EmulatorRequest::Type::AddIECDevice:
    case EmulatorRequest::Type::RemoveIECDevice:
    case EmulatorRequest::Type::SetIECConfiguration:
    case EmulatorRequest::Type::RestoreIECConfigurations:
    case EmulatorRequest::Type::TearOffIECPrinter:
        return EmulatorResponse::Result::InvalidParameter;
    }

    return EmulatorResponse::Result::InvalidParameter;
}


QSharedPointer<EmulatorResponse> EmulatorWorker::processIECRequest(const EmulatorRequest* ptrRequest)
{
    EmulatorResponse::Result result = EmulatorResponse::Result::Failed;

    QSharedPointer<EmulatorResponseIEC> response(new EmulatorResponseIEC(ptrRequest->id(), ptrRequest->type(), result));

    switch (ptrRequest->type())
    {
    case EmulatorRequest::Type::GetIECConfigurations:
    {
        const QList<QSharedPointer<IECDeviceConfiguration>> configurations = m_ptrIECSystem->configurations();
        const QList<IECDeviceType> availableTypes = m_ptrIECSystem->availableDeviceTypes();

        response->setConfigurations(configurations);
        response->setAvailableDeviceTypes(availableTypes);
        response->setAddableDeviceTypes(m_ptrIECSystem->addableDeviceTypes());

        for (const QSharedPointer<IECDeviceConfiguration>& ptrConfiguration : configurations)
        {
            if (ptrConfiguration.isNull() || ptrConfiguration->type() != IECDeviceType::FoundryDisk)
                continue;

            response->setDiskImageLoaded(ptrConfiguration->id(), m_ptrIECSystem->isDiskImageLoaded(ptrConfiguration->id()));
        }

        for (IECDeviceType type : availableTypes)
            response->setSupportedDeviceNumbers(type, m_ptrIECSystem->supportedDeviceNumbers(type));

        result = EmulatorResponse::Result::Success;
        break;
    }
    case EmulatorRequest::Type::AddIECDevice:
    {
        const EmulatorRequestAddIECDevice* ptrIECRequest = static_cast<const EmulatorRequestAddIECDevice*>(ptrRequest);

        const IECDeviceConfiguration::DeviceId id = m_ptrIECSystem->addDevice(ptrIECRequest->deviceType());

        if (id != 0)
        {
            response->setDeviceId(id);
            result = EmulatorResponse::Result::Success;
            publishIECDeviceEvent(EmulatorIECDeviceEvent::Type::Added, id);
        }
        break;
    }

    case EmulatorRequest::Type::RemoveIECDevice:
    {
        const EmulatorRequestRemoveIECDevice* ptrIECRequest = static_cast<const EmulatorRequestRemoveIECDevice*>(ptrRequest);

        if (m_ptrIECSystem->removeDevice(ptrIECRequest->deviceId()))
        {
            response->setDeviceId(ptrIECRequest->deviceId());
            result = EmulatorResponse::Result::Success;
            publishIECDeviceEvent(EmulatorIECDeviceEvent::Type::Removed, ptrIECRequest->deviceId());
        }
        break;
    }

    case EmulatorRequest::Type::SetIECConfiguration:
    {
        const EmulatorRequestSetIECConfiguration* ptrIECRequest = static_cast<const EmulatorRequestSetIECConfiguration*>(ptrRequest);

        const IECDeviceConfiguration& requestedConfiguration = ptrIECRequest->configuration();
        const QSharedPointer<IECDeviceConfiguration> ptrCurrentConfiguration = m_ptrIECSystem->configuration(requestedConfiguration.id());

        if (ptrCurrentConfiguration.isNull())
            break;

        const bool bConfigurationChanged = *ptrCurrentConfiguration != requestedConfiguration;

        QString operationMessage;
        if (m_ptrIECSystem->setConfiguration(requestedConfiguration, &operationMessage))
        {
            response->setDeviceId(requestedConfiguration.id());
            response->setMessage(operationMessage);

            if (requestedConfiguration.type() == IECDeviceType::FoundryDisk)
                response->setDiskImageLoaded(requestedConfiguration.id(), m_ptrIECSystem->isDiskImageLoaded(requestedConfiguration.id()));

            result = EmulatorResponse::Result::Success;

            if (bConfigurationChanged)
                publishIECDeviceEvent(EmulatorIECDeviceEvent::Type::ConfigurationChanged, requestedConfiguration.id());
        }

        break;
    }

    case EmulatorRequest::Type::RestoreIECConfigurations:
    {
        if (m_state != Emulator::State::PoweredOff)
        {
            result = EmulatorResponse::Result::InvalidState;
            break;
        }

        const EmulatorRequestRestoreIECConfigurations* ptrIECRequest = static_cast<const EmulatorRequestRestoreIECConfigurations*>(ptrRequest);

        if (!m_ptrIECSystem->restoreConfigurations(ptrIECRequest->configurations()))
            break;

        result = EmulatorResponse::Result::Success;

        publishIECDeviceEvent(EmulatorIECDeviceEvent::Type::Restored, 0);

        brerak;
    }

    case EmulatorRequest::Type::TearOffIECPrinter:
    {
        const EmulatorRequestTearOffIECPrinter* ptrIECRequest = static_cast<const EmulatorRequestTearOffIECPrinter*>(ptrRequest);

        if (!m_ptrIECSystem->tearOffPrinter(ptrIECRequest->deviceId()))
            break;

        response->setDeviceId(ptrIECRequest->deviceId());
        result = EmulatorResponse::Result::Success;
        break;
    }

    default:
        result = EmulatorResponse::Result::InvalidParameter;
        break;
    }

    response->setResult(result);
    return response;
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

void EmulatorWorker::publishIECDeviceEvent(EmulatorIECDeviceEvent::Type type, IECDeviceConfiguration::DeviceId deviceId)
{
    qDebug() << "EmulatorWorker: iecDeviceChanged"
             << "type:" << static_cast<int>(type)
             << "deviceId:" << deviceId;

    emit iecDeviceChanged(type, deviceId);
}

void EmulatorWorker::forwardIECEvents()
{
    const QList<IECSystem::PrinterOutputChange> printerChanges =
        m_ptrIECSystem->takePrinterOutputChanges();

    for (const IECSystem::PrinterOutputChange& change : printerChanges)
    {
        if (change.events.isEmpty())
            continue;

        emit printerOutputChanged(change.deviceId, change.events);
    }

    const QList<IECSystem::DriveAccessChange> driveChanges =
        m_ptrIECSystem->takeDriveAccessChanges();

    for (const IECSystem::DriveAccessChange& change : driveChanges)
    {
        if (change.events.isEmpty())
            continue;

        qDebug() << "EmulatorWorker: driveAccessChanged"
                 << "deviceId:" << change.deviceId
                 << "deviceNumber:" << change.deviceNumber
                 << "events:" << change.events.size();

        emit driveAccessChanged(change.deviceId, change.events);
    }
}
