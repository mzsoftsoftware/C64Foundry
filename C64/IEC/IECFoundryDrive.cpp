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
        *ptrChannelState = ChannelState();
        ptrChannelState->opened = true;
        ptrChannelState->type = event.secondaryAddress == 15
                                    ? ChannelType::DosCommand
                                    : ChannelType::File;
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
            if (ptrChannelState->type == ChannelType::File)
            {
                if (!ptrChannelState->inputBuffer.isEmpty())
                    ptrChannelState->fileName = ptrChannelState->inputBuffer;

                ptrChannelState->inputBuffer.clear();
            }
            else if (ptrChannelState->type == ChannelType::DosCommand)
            {
                if (!ptrChannelState->inputBuffer.isEmpty())
                    processDosCommand(event.deviceNumber, ptrDisk, ptrChannelState->inputBuffer);

                ptrChannelState->inputBuffer.clear();
            }
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

        if (ptrChannelState->type == ChannelType::BlockRead)
        {
            ptrChannelState->readingFile = false;

            if (ptrChannelState->blockBuffer.size() != 256)
            {
                ptrChannelState->blockBuffer.clear();

                if (!ptrDisk->readSector(
                        ptrChannelState->currentTrack,
                        ptrChannelState->currentSector,
                        ptrChannelState->blockBuffer))
                {
                    break;
                }
            }

            if (ptrChannelState->bufferPosition < 0 ||
                ptrChannelState->bufferPosition >= ptrChannelState->blockBuffer.size())
            {
                ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
                break;
            }

            response.type = IECProtocolResponse::Type::SendData;
            response.data = ptrChannelState->blockBuffer.mid(ptrChannelState->bufferPosition);
            response.lastBlock = true;

            ptrChannelState->bufferPosition = ptrChannelState->blockBuffer.size();
            break;
        }

        if (ptrChannelState->type == ChannelType::BlockWrite)
            break;

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

void IECFoundryDrive::processDosCommand(
    const quint8 deviceNumber,
    IECFoundryDisk* ptrDisk,
    const QByteArray& command)
{
    if (ptrDisk == nullptr)
        return;

    const QByteArray normalizedCommand = command.trimmed().toUpper();
    const int separatorPosition = normalizedCommand.indexOf(':');

    if (separatorPosition <= 0)
    {
        ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
        return;
    }

    const QByteArray commandName = normalizedCommand.left(separatorPosition);
    QByteArray arguments = normalizedCommand.mid(separatorPosition + 1);

    const bool bufferPointerCommand =
        commandName == QByteArrayLiteral("B-P");

    ChannelType channelType = ChannelType::None;

    if (commandName == QByteArrayLiteral("U1") ||
        commandName == QByteArrayLiteral("B-R"))
    {
        channelType = ChannelType::BlockRead;
    }
    else if (commandName == QByteArrayLiteral("U2") ||
             commandName == QByteArrayLiteral("B-W"))
    {
        channelType = ChannelType::BlockWrite;
    }
    else if (!bufferPointerCommand)
    {
        ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
        return;
    }

    arguments.replace(',', ' ');

    const QList<QByteArray> rawParameters = arguments.split(' ');
    QList<quint8> parameters;

    for (const QByteArray& rawParameter : rawParameters)
    {
        const QByteArray parameter = rawParameter.trimmed();

        if (parameter.isEmpty())
            continue;

        bool conversionOk = false;
        const uint value = parameter.toUInt(&conversionOk, 10);

        if (!conversionOk || value > 255)
        {
            ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
            return;
        }

        parameters.append(static_cast<quint8>(value));
    }

    const int expectedParameterCount = bufferPointerCommand ? 2 : 4;

    if (parameters.size() != expectedParameterCount)
    {
        ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
        return;
    }

    const quint8 channel = parameters.at(0);

    if (channel >= 15)
    {
        ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
        return;
    }

    ChannelState* ptrDataChannelState =
        findChannelState(deviceNumber, channel);

    if (ptrDataChannelState == nullptr || !ptrDataChannelState->opened)
    {
        ptrDisk->setDosStatus(70, QByteArrayLiteral("NO CHANNEL"));
        return;
    }

    if (bufferPointerCommand)
    {
        if (ptrDataChannelState->type != ChannelType::BlockRead &&
            ptrDataChannelState->type != ChannelType::BlockWrite)
        {
            ptrDisk->setDosStatus(70, QByteArrayLiteral("NO CHANNEL"));
            return;
        }

        ptrDataChannelState->bufferPosition = parameters.at(1);

        ptrDisk->setDosStatus(0, QByteArrayLiteral("OK"));
        return;
    }

    const quint8 drive = parameters.at(1);
    const quint8 track = parameters.at(2);
    const quint8 sector = parameters.at(3);

    if (drive != 0)
    {
        ptrDisk->setDosStatus(30, QByteArrayLiteral("SYNTAX ERROR"));
        return;
    }

    ptrDataChannelState->type = channelType;
    ptrDataChannelState->readingFile = false;
    ptrDataChannelState->fileName.clear();
    ptrDataChannelState->inputBuffer.clear();
    ptrDataChannelState->blockBuffer.clear();
    ptrDataChannelState->bufferPosition = 0;
    ptrDataChannelState->currentTrack = track;
    ptrDataChannelState->currentSector = sector;

    ptrDisk->setDosStatus(0, QByteArrayLiteral("OK"));
}
