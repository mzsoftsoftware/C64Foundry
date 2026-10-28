#include "IECFoundryDrive.h"

#include <QFile>
#include <QFileInfo>

#include "IECFoundryDisk.h"


struct IECFoundryDrive::DriveState
{
    IECFoundryDisk* ptrDisk = nullptr;
};

IECFoundryDrive::IECFoundryDrive()
{
}
IECFoundryDrive::~IECFoundryDrive()
{
    qDeleteAll(m_drives);
    m_drives.clear();
}

bool IECFoundryDrive::registerDisk(IECFoundryDisk* ptrDisk)
{
    Q_ASSERT(ptrDisk);

    const quint8 deviceNumber = ptrDisk->deviceNumber();
    if (deviceNumber < 8 || deviceNumber > 11)
        return false;

    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);
    if (ptrState != nullptr)
    {
        if (ptrState->ptrDisk == ptrDisk)
            return true;

        if (ptrState->ptrDisk != nullptr)
            return false;

        ptrState->ptrDisk = ptrDisk;
        return true;
    }

    ptrState = new DriveState();
    ptrState->ptrDisk = ptrDisk;
    m_drives.insert(deviceNumber, ptrState);

    return true;
}


void IECFoundryDrive::unregisterDisk(IECFoundryDisk* ptrDisk)
{
    Q_ASSERT(ptrDisk);

    DriveState* ptrState = m_drives.value(ptrDisk->deviceNumber(), nullptr);
    if (ptrState == nullptr)
        return;

    if (ptrState->ptrDisk != ptrDisk)
        return;

    // Den Laufwerkszustand behalten, aber die aktive IEC-Disk lösen.
    ptrState->ptrDisk = nullptr;
}


int IECFoundryDrive::diskCount() const
{
    int count = 0;
    for (DriveState* ptrState : m_drives)
    {
        if (ptrState != nullptr && ptrState->ptrDisk != nullptr)
            ++count;
    }
    return count;
}


bool IECFoundryDrive::hasDisk(quint8 deviceNumber) const
{
    const DriveState* ptrState = m_drives.value(deviceNumber, nullptr);
    return ptrState != nullptr && ptrState->ptrDisk != nullptr;
}
