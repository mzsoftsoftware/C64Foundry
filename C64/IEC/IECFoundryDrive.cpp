#include "IECFoundryDrive.h"

#include "IECFoundryDisk.h"

#include <QSharedPointer>


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

        m_configuration = newConfiguration;

        for (int slotIndex = 0; slotIndex < m_configuration.slotCount(); ++slotIndex)
        {
            IECFoundryDiskConfiguration* ptrDiskConfiguration = m_configuration.diskConfiguration(slotIndex);

            if (ptrDiskConfiguration == nullptr)
                continue;

            ptrDiskConfiguration->setDiskId(m_nextDiskId++);
            ptrDiskConfiguration->setSlotIndex(slotIndex);
            ptrDiskConfiguration->setDeviceNumber(static_cast<quint8>(m_configuration.startDeviceNumber() + slotIndex));

            m_disks.append(new IECFoundryDisk(this, *ptrDiskConfiguration));
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


bool IECFoundryDrive::insertImage(const quint8 deviceNumber, const QString& fileName)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    return ptrDisk != nullptr && ptrDisk->insertImage(fileName);
}


bool IECFoundryDrive::ejectImage(const quint8 deviceNumber)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    return ptrDisk != nullptr && ptrDisk->ejectImage();
}


bool IECFoundryDrive::saveImage(const quint8 deviceNumber)
{
    IECFoundryDisk* ptrDisk = findDisk(deviceNumber);

    return ptrDisk != nullptr && ptrDisk->saveImage();
}


void IECFoundryDrive::reset()
{
    m_events.clear();

    for (IECFoundryDisk* ptrDisk : m_disks)
        ptrDisk->takeSectorAccessEvents();
}


void IECFoundryDrive::processEvent(const IECProtocolEvent& event, IECProtocolResponse& response)
{
    Q_UNUSED(event);
    Q_UNUSED(response);
}


void IECFoundryDrive::collectDiskEvents()
{
    for (int slotIndex = 0; slotIndex < m_disks.size(); ++slotIndex)
    {
        IECFoundryDisk* ptrDisk = m_disks.at(slotIndex);
        QQueue<IECFoundryDisk::SectorAccessStatus> diskEvents = ptrDisk->takeSectorAccessEvents();

        while (!diskEvents.isEmpty())
        {
            const IECFoundryDisk::SectorAccessStatus status = diskEvents.dequeue();

            QSharedPointer<IECDriveAccessEvent> event(new IECDriveAccessEvent());
            event->deviceId = id();
            event->deviceNumber = static_cast<quint8>(m_configuration.startDeviceNumber() + slotIndex);
            event->operation = status.operation == IECFoundryDisk::SectorAccessOperation::Read
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
