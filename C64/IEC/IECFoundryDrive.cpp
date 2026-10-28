#include "IECFoundryDrive.h"

#include <QFile>
#include <QSaveFile>

#include "IECFoundryDisk.h"


struct IECFoundryDrive::DriveState
{
    IECFoundryDisk* ptrDisk = nullptr;
    QString imageFileName;
    QByteArray imageData;
    bool imageModified = false;
};
int IECFoundryDrive::sectorsOnTrack(int track)
{
    if (track < 1 || track > 35)
        return 0;
    if (track <= 17)
        return 21;
    if (track <= 24)
        return 19;
    if (track <= 30)
        return 18;
    return 17;
}
int IECFoundryDrive::sectorOffset(int track, int sector)
{
    if (track < 1 || track > 35)
        return -1;
    const int sectors = sectorsOnTrack(track);
    if (sector < 0 || sector >= sectors)
        return -1;
    int offset = 0;
    for (int currentTrack = 1; currentTrack < track; ++currentTrack)
        offset += sectorsOnTrack(currentTrack);
    return (offset + sector) * 256;
}

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

    const quint8 deviceNumber = ptrDisk->deviceNumber();
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);
    if (ptrState == nullptr)
        return;

    if (ptrState->ptrDisk != ptrDisk)
        return;

    m_drives.remove(deviceNumber);
    delete ptrState;
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

bool IECFoundryDrive::insertImage(quint8 deviceNumber, const QString& fileName, QString* errorMessage)
{
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
        return false;

    if (errorMessage != nullptr)
        errorMessage->clear();

    // Ein bereits geladenes Image wird immer zuerst ausgeworfen.
    // Das Speichern übernimmt ausschließlich ejectImage().
    if (hasImage(deviceNumber))
        ejectImage(deviceNumber, errorMessage);

    // Ein leerer Pfad bedeutet: nur auswerfen.
    if (fileName.isEmpty())
        return true;

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        if (errorMessage != nullptr)
        {
            if (!errorMessage->isEmpty())
                *errorMessage += QChar('\n');

            *errorMessage += QStringLiteral("Image konnte nicht geöffnet werden: %1\n%2").arg(fileName, file.errorString());
        }

        return true;
    }

    const QByteArray imageData = file.readAll();

    if (file.error() != QFileDevice::NoError)
    {
        if (errorMessage != nullptr)
        {
            if (!errorMessage->isEmpty())
                *errorMessage += QChar('\n');

            *errorMessage += QStringLiteral("Fehler beim Lesen des Images: %1\n%2").arg(fileName, file.errorString());
        }

        return true;
    }

    ptrState->imageFileName = fileName;
    ptrState->imageData = imageData;
    ptrState->imageModified = false;

    return true;
}
bool IECFoundryDrive::ejectImage(quint8 deviceNumber, QString* errorMessage)
{
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
        return false;

    if (errorMessage != nullptr)
        errorMessage->clear();

    // Ein geändertes Image wird vor dem Auswerfen gespeichert.
    if (ptrState->imageModified)
        saveImage(deviceNumber, errorMessage);

    // Auch bei einem Speicherfehler wird das Image ausgeworfen.
    ptrState->imageFileName.clear();
    ptrState->imageData.clear();
    ptrState->imageModified = false;

    return true;
}
bool IECFoundryDrive::saveImage(quint8 deviceNumber, QString* errorMessage)
{
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr || ptrState->imageFileName.isEmpty() || ptrState->imageData.isNull())
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Es ist kein Image zum Speichern geladen.");

        return false;
    }

    QSaveFile file(ptrState->imageFileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Image konnte nicht zum Schreiben geöffnet werden: %1\n%2").arg(ptrState->imageFileName, file.errorString());

        return false;
    }

    if (file.write(ptrState->imageData) != ptrState->imageData.size())
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Fehler beim Schreiben des Images: %1\n%2").arg(ptrState->imageFileName, file.errorString());

        file.cancelWriting();
        return false;
    }

    if (!file.commit())
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Image konnte nicht gespeichert werden: %1\n%2").arg(ptrState->imageFileName, file.errorString());

        return false;
    }

    ptrState->imageModified = false;
    return true;
}

bool IECFoundryDrive::hasImage(quint8 deviceNumber) const
{
    const DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    return ptrState != nullptr &&
           ptrState->ptrDisk != nullptr &&
           !ptrState->imageFileName.isEmpty() &&
           !ptrState->imageData.isNull();
}
QString IECFoundryDrive::imageFileName(quint8 deviceNumber) const
{
    const DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
        return QString();

    return ptrState->imageFileName;
}

bool IECFoundryDrive::readSector(quint8 deviceNumber, quint8 track, quint8 sector, QByteArray& data)
{
    Q_UNUSED(deviceNumber);
    Q_UNUSED(track);
    Q_UNUSED(sector);
    Q_UNUSED(data);

    return false;
}


bool IECFoundryDrive::writeSector(quint8 deviceNumber, quint8 track, quint8 sector, const QByteArray& data)
{
    Q_UNUSED(deviceNumber);
    Q_UNUSED(track);
    Q_UNUSED(sector);
    Q_UNUSED(data);

    return false;
}
