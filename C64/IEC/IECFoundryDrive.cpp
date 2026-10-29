#include "IECFoundryDrive.h"

#include <QFile>
#include <QSaveFile>
#include <QSet>

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
bool IECFoundryDrive::appendBasicLine(QByteArray& program, quint16& address, quint16 lineNumber, const QByteArray& content)
{
    const quint32 nextAddress = static_cast<quint32>(address) + 2U + 2U + static_cast<quint32>(content.size()) + 1U;

    if (nextAddress > 0xFFFFU)
        return false;

    program.append(static_cast<char>(nextAddress & 0xFFU));
    program.append(static_cast<char>((nextAddress >> 8) & 0xFFU));

    program.append(static_cast<char>(lineNumber & 0xFFU));
    program.append(static_cast<char>((lineNumber >> 8) & 0xFFU));

    program.append(content);
    program.append('\0');

    address = static_cast<quint16>(nextAddress);

    return true;
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

bool IECFoundryDrive::readSector(const quint8 deviceNumber, const quint8 track, const quint8 sector, QByteArray& data)
{
    data.clear();

    const DriveState* ptrState = m_drives.value(deviceNumber, nullptr);
    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
    {
        m_lastError = QStringLiteral("Das Laufwerk ist nicht vorhanden.");
        return false;
    }
    if (ptrState->imageFileName.isEmpty() || ptrState->imageData.isNull())
    {
        m_lastError = QStringLiteral("Es ist kein Disketten-Image geladen.");
        return false;
    }

    const int offset = sectorOffset(track, sector);
    if (offset < 0)
    {
        m_lastError = QStringLiteral("Ungültiger Track oder Sektor.");
        return false;
    }
    constexpr int SECTOR_SIZE = 256;
    if (offset + SECTOR_SIZE > ptrState->imageData.size())
    {
        m_lastError = QStringLiteral("Der Sektor liegt außerhalb des Disketten-Images.");
        return false;
    }

    data = ptrState->imageData.mid(offset, SECTOR_SIZE);
    return data.size() == SECTOR_SIZE;
}

bool IECFoundryDrive::writeSector(quint8 deviceNumber, quint8 track, quint8 sector, const QByteArray& data)
{
    Q_UNUSED(deviceNumber);
    Q_UNUSED(track);
    Q_UNUSED(sector);
    Q_UNUSED(data);

    return false;
}

bool IECFoundryDrive::createDirectoryProgram(const quint8 deviceNumber, QByteArray& program, QString* errorMessage)
{
    program.clear();

    if (errorMessage != nullptr)
        errorMessage->clear();
    if (!hasImage(deviceNumber))
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Kein Disketten-Image geladen.");
        return false;
    }

    QByteArray bam;
    if (!readSector(deviceNumber, 18, 0, bam) || bam.size() != 256)
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Der BAM-Sektor der Diskette konnte nicht gelesen werden.");
        return false;
    }

    // PRG-Ladeadresse: $0801.
    program.append(static_cast<char>(0x01));
    program.append(static_cast<char>(0x08));
    quint16 address = 0x0801;
    QByteArray diskName = bam.mid(144, 16);
    while (!diskName.isEmpty() && (static_cast<quint8>(diskName.back()) == 0xA0 || diskName.back() == ' '))
    {
        diskName.chop(1);
    }

    QByteArray header;
    header.append('"');
    header.append(diskName);
    header.append('"');
    header.append(' ');
    header.append(bam.mid(161, 2));
    header.append(' ');
    header.append(bam.mid(164, 2));
    if (!appendBasicLine(program, address, 0, header))
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Das Directory ist zu groß für ein BASIC-Programm.");
        program.clear();
        return false;
    }

    quint8 track = 18;
    quint8 sector = 1;
    QSet<quint16> visitedSectors;
    while (track != 0)
    {
        const quint16 sectorKey = static_cast<quint16>((static_cast<quint16>(track) << 8) | sector);
        if (visitedSectors.contains(sectorKey))
        {
            if (errorMessage != nullptr)
                *errorMessage = QStringLiteral("Die Directory-Sektorkette enthält einen Zyklus.");
            program.clear();
            return false;
        }

        visitedSectors.insert(sectorKey);
        QByteArray directorySector;
        if (!readSector(deviceNumber, track, sector, directorySector) || directorySector.size() != 256)
        {
            if (errorMessage != nullptr)
                *errorMessage = QStringLiteral("Ein Directory-Sektor konnte nicht gelesen werden.");
            program.clear();
            return false;
        }

        const quint8 nextTrack = static_cast<quint8>(directorySector.at(0));
        const quint8 nextSector = static_cast<quint8>(directorySector.at(1));
        for (int entryOffset = 2; entryOffset + 31 < directorySector.size(); entryOffset += 32)
        {
            const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));
            if ((fileType & 0x07U) == 0)
                continue;

            const quint16 blocks = static_cast<quint16>(static_cast<quint8>(directorySector.at(entryOffset + 30)) | (static_cast<quint16>(static_cast<quint8>(directorySector.at(entryOffset + 31))) << 8));
            QByteArray fileName = directorySector.mid(entryOffset + 3, 16);
            while (!fileName.isEmpty() && (static_cast<quint8>(fileName.back()) == 0xA0 || fileName.back() == ' '))
            {
                fileName.chop(1);
            }

            QByteArray typeName;
            switch (fileType & 0x07U)
            {
            case 1:
                typeName = "SEQ";
                break;
            case 2:
                typeName = "PRG";
                break;
            case 3:
                typeName = "USR";
                break;
            case 4:
                typeName = "REL";
                break;
            default:
                typeName = "???";
                break;
            }

            if ((fileType & 0x80U) == 0)
                typeName.append('*');
            if ((fileType & 0x40U) != 0)
                typeName.append('<');

            QByteArray line = QByteArray::number(blocks).rightJustified(5, ' ');
            line.append(" \"");
            line.append(fileName);
            line.append("\" ");
            line.append(typeName);

            if (!appendBasicLine(program, address, blocks, line))
            {
                if (errorMessage != nullptr)
                    *errorMessage = QStringLiteral("Das Directory ist zu groß für ein BASIC-Programm.");
                program.clear();
                return false;
            }
        }

        track = nextTrack;
        sector = nextSector;
    }

    quint16 freeBlocks = 0;
    for (int currentTrack = 1; currentTrack <= 35; ++currentTrack)
    {
        const int bamOffset = 4 + ((currentTrack - 1) * 4);
        freeBlocks += static_cast<quint8>(bam.at(bamOffset));
    }

    const QByteArray freeBlocksLine = QByteArrayLiteral("BLOCKS FREE.");
    if (!appendBasicLine(program, address, freeBlocks, freeBlocksLine))
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Das Directory ist zu groß für ein BASIC-Programm.");
        program.clear();
        return false;
    }

    // Ende des BASIC-Programms: Nullzeiger.
    program.append('\0');
    program.append('\0');

    return true;
}
