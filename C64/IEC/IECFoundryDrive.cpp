#include "IECFoundryDrive.h"

#include "IECFoundryDisk.h"

#include <QSharedPointer>
#include <QQueue>


IECFoundryDrive::IECFoundryDrive(const DeviceId id)
    : IECDevice(id)
{
    m_configuration.setId(id);
}


IECFoundryDrive::~IECFoundryDrive()
{
    for (IECFoundryDisk* ptrDisk : m_disks)
        delete ptrDisk;

    m_disks.clear();
}


bool IECFoundryDrive::setConfiguration(const IECFoundryDriveConfiguration& configuration)
{
    if (configuration.slotCount() > IECFoundryDriveConfiguration::MaximumDiskCount)
        return false;

    if (configuration.startDeviceNumber() < 4 || configuration.endDeviceNumber() > 30)
        return false;

    const bool rebuild = configuration.startDeviceNumber() != m_configuration.startDeviceNumber() ||
                         configuration.slotCount() != m_configuration.slotCount();

    IECFoundryDriveConfiguration newConfiguration = configuration;
    newConfiguration.setId(id());

    if (rebuild)
    {
        for (IECFoundryDisk* ptrDisk : m_disks)
            delete ptrDisk;

        m_disks.clear();
        m_channelStates.clear();

        m_configuration = newConfiguration;

        for (int slotIndex = 0; slotIndex < m_configuration.slotCount(); ++slotIndex)
        {
            IECFoundryDiskConfiguration* ptrDiskConfiguration = m_configuration.diskConfiguration(slotIndex);

            if (ptrDiskConfiguration == nullptr)
                continue;

            ptrDiskConfiguration->setDiskId(m_nextDiskId++);
            ptrDiskConfiguration->setSlotIndex(slotIndex);
            ptrDiskConfiguration->setDeviceNumber(static_cast<quint8>(m_configuration.startDeviceNumber() + slotIndex));

            m_disks.append(new IECFoundryDisk(*ptrDiskConfiguration));

            DiskChannelState channelState;
            channelState.channels.resize(16);
            m_channelStates.append(channelState);
        }

        return true;
    }

    bool success = true;

    for (int slotIndex = 0; slotIndex < m_disks.size(); ++slotIndex)
    {
        IECFoundryDiskConfiguration* ptrDiskConfiguration = newConfiguration.diskConfiguration(slotIndex);

        if (ptrDiskConfiguration == nullptr)
            continue;

        IECFoundryDisk* ptrDisk = m_disks.at(slotIndex);

        ptrDiskConfiguration->setDiskId(ptrDisk->diskId());
        ptrDiskConfiguration->setSlotIndex(slotIndex);
        ptrDiskConfiguration->setDeviceNumber(static_cast<quint8>(newConfiguration.startDeviceNumber() + slotIndex));

        if (!ptrDisk->setConfiguration(*ptrDiskConfiguration))
            success = false;
    }

    m_configuration = newConfiguration;

    return success;
}


IECFoundryDriveConfiguration IECFoundryDrive::configuration() const
{
    IECFoundryDriveConfiguration result = m_configuration;
    result.setId(id());

    return result;
}


int IECFoundryDrive::diskCount() const
{
    return m_disks.size();
}


IECFoundryDisk* IECFoundryDrive::disk(const int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= m_disks.size())
        return nullptr;

    return m_disks.at(slotIndex);
}


IECFoundryDisk* IECFoundryDrive::diskByDeviceNumber(const quint8 deviceNumber) const
{
    return findDisk(deviceNumber);
}


IECFoundryDisk* IECFoundryDrive::findDisk(const quint8 deviceNumber) const
{
    const int slotIndex = static_cast<int>(deviceNumber) - m_configuration.startDeviceNumber();

    if (slotIndex < 0 || slotIndex >= m_disks.size())
        return nullptr;

    return m_disks.at(slotIndex);
}


IECFoundryDrive::ChannelState* IECFoundryDrive::findChannelState(
    const quint8 deviceNumber, const quint8 channel)
{
    if (channel > 15)
        return nullptr;

    const int slotIndex = static_cast<int>(deviceNumber) - m_configuration.startDeviceNumber();

    if (slotIndex < 0 || slotIndex >= m_channelStates.size())
        return nullptr;

    DiskChannelState& diskState = m_channelStates[slotIndex];

    if (channel >= diskState.channels.size())
        return nullptr;

    return &diskState.channels[channel];
}


bool IECFoundryDrive::setDiskEnabled(
    const quint8 deviceNumber, const bool enabled)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    if (ptrDisk == nullptr)
        return false;

    IECFoundryDiskConfiguration* ptrConfiguration =
        m_configuration.diskConfiguration(ptrDisk->slotIndex());

    if (ptrConfiguration == nullptr)
        return false;

    ptrDisk->setEnabled(enabled);
    ptrConfiguration->setEnabled(enabled);

    return true;
}


bool IECFoundryDrive::insertImage(const quint8 deviceNumber, const QString& fileName)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    if (ptrDisk == nullptr || !ptrDisk->insertImage(fileName))
        return false;

    IECFoundryDiskConfiguration* ptrConfiguration =
        m_configuration.diskConfiguration(ptrDisk->slotIndex());

    if (ptrConfiguration == nullptr)
        return false;

    ptrConfiguration->setImageFileName(ptrDisk->imageFileName());

    return true;
}


bool IECFoundryDrive::ejectImage(const quint8 deviceNumber)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    if (ptrDisk == nullptr || !ptrDisk->ejectImage())
        return false;

    IECFoundryDiskConfiguration* ptrConfiguration =
        m_configuration.diskConfiguration(ptrDisk->slotIndex());

    if (ptrConfiguration == nullptr)
        return false;

    ptrConfiguration->setImageFileName(QString());

    return true;
}

bool IECFoundryDrive::saveImage(const quint8 deviceNumber)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    return ptrDisk != nullptr && ptrDisk->saveImage();
}


void IECFoundryDrive::reset()
{
    m_events.clear();

    for (DiskChannelState& diskState : m_channelStates)
    {
        for (ChannelState& channelState : diskState.channels)
            channelState = ChannelState();
    }

    for (IECFoundryDisk* ptrDisk : m_disks)
    {
        ptrDisk->takeSectorAccessEvents();
        ptrDisk->takeImageEvents();
    }
}


void IECFoundryDrive::processEvent(
    const IECProtocolEvent& event,
    IECProtocolResponse& response)
{
    response.type = IECProtocolResponse::Type::None;
    response.data.clear();
    response.lastBlock = true;

    IECFoundryDisk* ptrDisk = findDisk(event.deviceNumber);

    if (ptrDisk == nullptr || !ptrDisk->enabled())
        return;

    ChannelState* ptrChannelState =
        findChannelState(event.deviceNumber, event.secondaryAddress);

    if (ptrChannelState == nullptr)
        return;

    switch (event.type)
    {
    case IECProtocolEvent::Type::DeviceSelected:
        break;

    case IECProtocolEvent::Type::Open:
        ptrChannelState->opened = true;
        ptrChannelState->readingFile = false;
        ptrChannelState->fileName.clear();
        ptrChannelState->inputBuffer.clear();
        ptrChannelState->currentTrack = 0;
        ptrChannelState->currentSector = 0;
        break;

    case IECProtocolEvent::Type::Close:
        *ptrChannelState = ChannelState();
        break;

    case IECProtocolEvent::Type::ByteReceived:
        if (event.mode == IECProtocolEvent::Mode::Listen)
            ptrChannelState->inputBuffer.append(static_cast<char>(event.value));
        break;

    case IECProtocolEvent::Type::EndOfTransmission:
        if (event.mode == IECProtocolEvent::Mode::Listen)
        {
            if (!ptrChannelState->inputBuffer.isEmpty())
                ptrChannelState->fileName = ptrChannelState->inputBuffer;

            ptrChannelState->inputBuffer.clear();
        }
        break;

    case IECProtocolEvent::Type::SecondaryAddressReceived:
        if (event.mode != IECProtocolEvent::Mode::Talk)
            break;

        if (event.secondaryAddress == 15)
        {
            if (!ptrDisk->imageValid())
                ptrDisk->setDosStatus(74, QByteArrayLiteral("DRIVE NOT READY"));

            response.type = IECProtocolResponse::Type::SendData;
            response.data = ptrDisk->takeDosStatus();
            response.lastBlock = true;
            break;
        }

        ptrChannelState->readingFile = false;

        if (!ptrDisk->imageValid())
        {
            ptrDisk->setDosStatus(74, QByteArrayLiteral("DRIVE NOT READY"));
            break;
        }

        {
            const QByteArray requestedName =
                ptrChannelState->fileName.trimmed().toUpper();

            if (requestedName == QByteArrayLiteral("$"))
            {
                QByteArray directoryProgram;

                if (!ptrDisk->createDirectoryProgram(directoryProgram))
                    break;

                response.type = IECProtocolResponse::Type::SendData;
                response.data = directoryProgram;
                response.lastBlock = true;
                break;
            }

            IECFoundryDisk::FileOpenResult openResult;

            if (!ptrDisk->openFile(requestedName, openResult) || openResult.empty)
                break;

            ptrChannelState->currentTrack = openResult.firstTrack;
            ptrChannelState->currentSector = openResult.firstSector;
            ptrChannelState->readingFile = true;

            prepareNextFileBlock(ptrDisk, *ptrChannelState, response);
        }
        break;

    case IECProtocolEvent::Type::RequestNextDataBlock:
        if (event.mode == IECProtocolEvent::Mode::Talk && ptrChannelState->readingFile)
            prepareNextFileBlock(ptrDisk, *ptrChannelState, response);
        break;

    case IECProtocolEvent::Type::DeviceDeselected:
        // Geöffnete Kanäle bleiben über UNLISTEN und UNTALK hinweg erhalten.
        break;
    }
}

void IECFoundryDrive::collectDiskEvents()
{
    for (int slotIndex = 0; slotIndex < m_disks.size(); ++slotIndex)
    {
        IECFoundryDisk* ptrDisk = m_disks.at(slotIndex);

        const quint8 deviceNumber =
            static_cast<quint8>(m_configuration.startDeviceNumber() + slotIndex);

        QQueue<IECDriveImageEvent> imageEvents = ptrDisk->takeImageEvents();

        while (!imageEvents.isEmpty())
        {
            IECDriveImageEvent imageEvent = imageEvents.dequeue();

            imageEvent.deviceId = id();
            imageEvent.deviceNumber = deviceNumber;

            m_events.append(IECDeviceEventPtr(
                new IECDriveImageEvent(imageEvent)));
        }

        QQueue<IECFoundryDisk::SectorAccessStatus> diskEvents =
            ptrDisk->takeSectorAccessEvents();

        while (!diskEvents.isEmpty())
        {
            const IECFoundryDisk::SectorAccessStatus status =
                diskEvents.dequeue();

            QSharedPointer<IECDriveAccessEvent> event(
                new IECDriveAccessEvent());

            event->deviceId = id();
            event->deviceNumber = deviceNumber;
            event->operation =
                status.operation == IECFoundryDisk::SectorAccessOperation::Read
                    ? IECDriveAccessEvent::Operation::Read
                    : IECDriveAccessEvent::Operation::Write;
            event->track = status.track;
            event->sector = status.sector;
            event->success = status.success;
            event->errorMessage = status.errorMessage;

            m_events.append(event);
        }
    }
}


IECDeviceEventList IECFoundryDrive::takeEvents()
{
    collectDiskEvents();

    IECDeviceEventList events;
    events.swap(m_events);

    return events;
}

bool IECFoundryDrive::prepareNextFileBlock(
    IECFoundryDisk* ptrDisk,
    ChannelState& channelState,
    IECProtocolResponse& response)
{
    if (ptrDisk == nullptr || !channelState.readingFile)
        return false;

    IECFoundryDisk::FileDataBlock block;

    if (!ptrDisk->readFileBlock(
            channelState.currentTrack,
            channelState.currentSector,
            block))
    {
        channelState.readingFile = false;
        return false;
    }

    if (block.data.isEmpty())
    {
        channelState.readingFile = false;
        return false;
    }

    response.type = IECProtocolResponse::Type::SendData;
    response.data = block.data;
    response.lastBlock = block.isLast;

    if (block.isLast)
    {
        channelState.readingFile = false;
    }
    else
    {
        channelState.currentTrack = block.nextTrack;
        channelState.currentSector = block.nextSector;
    }

    return true;
}
