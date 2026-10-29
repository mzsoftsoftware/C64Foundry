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
    {
        m_qstrLastError = QStringLiteral("Das Directory ist zu groß für ein BASIC-Programm.");
        return false;
    }

    program.append(static_cast<char>(nextAddress & 0xFFU));
    program.append(static_cast<char>((nextAddress >> 8) & 0xFFU));

    program.append(static_cast<char>(lineNumber & 0xFFU));
    program.append(static_cast<char>((lineNumber >> 8) & 0xFFU));

    program.append(content);
    program.append('\0');

    address = static_cast<quint16>(nextAddress);

    return true;
}
QByteArray IECFoundryDrive::normalizeDiskFileName(QByteArray fileName)
{
    while (!fileName.isEmpty() && (static_cast<quint8>(fileName.back()) == 0xA0 || fileName.back() == ' '))
    {
        fileName.chop(1);
    }
    return fileName.toUpper();
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

bool IECFoundryDrive::insertImage(quint8 deviceNumber, const QString& fileName)
{
    m_qstrLastError.clear();
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
    {
        m_qstrLastError = QStringLiteral("Das Laufwerk ist nicht vorhanden.");
        return false;
    }

    // Ein bereits geladenes Image wird immer zuerst ausgeworfen.
    // Das Speichern übernimmt ausschließlich ejectImage().
    if (hasImage(deviceNumber))
        ejectImage(deviceNumber);

    // Ein leerer Pfad bedeutet: nur auswerfen.
    if (fileName.isEmpty())
        return true;

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        m_qstrLastError = QStringLiteral("Image konnte nicht geöffnet werden: %1\n%2").arg(fileName, file.errorString());
        return false;
    }

    const QByteArray imageData = file.readAll();

    if (file.error() != QFileDevice::NoError)
    {
        m_qstrLastError = QStringLiteral("Fehler beim Lesen des Images: %1\n%2").arg(fileName, file.errorString());
        return false;
    }

    ptrState->imageFileName = fileName;
    ptrState->imageData = imageData;
    ptrState->imageModified = false;

    return true;
}
bool IECFoundryDrive::ejectImage(quint8 deviceNumber)
{
    m_qstrLastError.clear();
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
    {
        m_qstrLastError = QStringLiteral("Das Laufwerk ist nicht vorhanden.");
        return false;
    }

    // Ein geändertes Image wird vor dem Auswerfen gespeichert.
    if (ptrState->imageModified && !saveImageInternal(deviceNumber))
        return false;

    // Auch bei einem Speicherfehler wird das Image ausgeworfen.
    ptrState->imageFileName.clear();
    ptrState->imageData.clear();
    ptrState->imageModified = false;

    return true;
}
bool IECFoundryDrive::saveImage(quint8 deviceNumber)
{
    m_qstrLastError.clear();
    return saveImageInternal(deviceNumber);
}
bool IECFoundryDrive::saveImageInternal(quint8 deviceNumber)
{
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr || ptrState->imageFileName.isEmpty() || ptrState->imageData.isNull())
    {
        m_qstrLastError = QStringLiteral("Es ist kein Image zum Speichern geladen.");
        return false;
    }

    QSaveFile file(ptrState->imageFileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        m_qstrLastError = QStringLiteral("Image konnte nicht zum Schreiben geöffnet werden: %1\n%2").arg(ptrState->imageFileName, file.errorString());
        return false;
    }

    if (file.write(ptrState->imageData) != ptrState->imageData.size())
    {
        m_qstrLastError = QStringLiteral("Fehler beim Schreiben des Images: %1\n%2").arg(ptrState->imageFileName, file.errorString());
        file.cancelWriting();
        return false;
    }

    if (!file.commit())
    {
        m_qstrLastError = QStringLiteral("Image konnte nicht gespeichert werden: %1\n%2").arg(ptrState->imageFileName, file.errorString());
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
    m_qstrLastError.clear();
    data.clear();

    const DriveState* ptrState = m_drives.value(deviceNumber, nullptr);
    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
    {
        m_qstrLastError = QStringLiteral("Das Laufwerk ist nicht vorhanden.");
        return false;
    }
    if (ptrState->imageFileName.isEmpty() || ptrState->imageData.isNull())
    {
        m_qstrLastError = QStringLiteral("Es ist kein Disketten-Image geladen.");
        return false;
    }

    const int offset = sectorOffset(track, sector);
    if (offset < 0)
    {
        m_qstrLastError = QStringLiteral("Ungültiger Track oder Sektor.");
        return false;
    }
    constexpr int SECTOR_SIZE = 256;
    if (offset + SECTOR_SIZE > ptrState->imageData.size())
    {
        m_qstrLastError = QStringLiteral("Der Sektor liegt außerhalb des Disketten-Images.");
        return false;
    }

    data = ptrState->imageData.mid(offset, SECTOR_SIZE);
    if (data.size() != SECTOR_SIZE)
    {
        data.clear();
        m_qstrLastError = QStringLiteral("Der Sektor konnte nicht vollständig gelesen werden.");
        return false;
    }

    return true;
}

bool IECFoundryDrive::writeSector(quint8 deviceNumber, quint8 track, quint8 sector, const QByteArray& data)
{
    Q_UNUSED(deviceNumber);
    Q_UNUSED(track);
    Q_UNUSED(sector);
    Q_UNUSED(data);

    m_qstrLastError.clear();
    m_qstrLastError = QStringLiteral("Das Schreiben von Sektoren ist noch nicht implementiert.");

    return false;
}

bool IECFoundryDrive::createDirectoryProgram(const quint8 deviceNumber, QByteArray& program)
{
    m_qstrLastError.clear();
    program.clear();

    if (!hasImage(deviceNumber))
    {
        m_qstrLastError = QStringLiteral("Kein Disketten-Image geladen.");
        return false;
    }

    QByteArray bam;
    if (!readSector(deviceNumber, 18, 0, bam) || bam.size() != 256)
        return false;

    if (bam.size() != 256)
    {
        m_qstrLastError = QStringLiteral("Der BAM-Sektor der Diskette hat eine ungültige Größe.");
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
            m_qstrLastError = QStringLiteral("Die Directory-Sektorkette enthält einen Zyklus.");
            program.clear();
            return false;
        }

        visitedSectors.insert(sectorKey);
        QByteArray directorySector;
        if (!readSector(deviceNumber, track, sector, directorySector) || directorySector.size() != 256)
        {
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
        program.clear();
        return false;
    }

    // Ende des BASIC-Programms: Nullzeiger.
    program.append('\0');
    program.append('\0');

    return true;
}

bool IECFoundryDrive::readFile(const quint8 deviceNumber, const QByteArray& fileName, QByteArray& data)
{
    m_qstrLastError.clear();
    data.clear();

    if (!hasImage(deviceNumber))
    {
        m_qstrLastError = QStringLiteral("Kein Disketten-Image geladen.");
        return false;
    }
    const QByteArray requestedName = normalizeDiskFileName(fileName);
    if (requestedName.isEmpty())
    {
        m_qstrLastError = QStringLiteral("Es wurde kein Dateiname angegeben.");
        return false;
    }

    QByteArray directorySector;
    if (!readSector(deviceNumber, 18, 1, directorySector))
        return false;

    quint8 directoryTrack = 18;
    quint8 directorySectorNumber = 1;
    quint8 fileTrack = 0;
    quint8 fileSectorNumber = 0;
    bool fileFound = false;
    QSet<quint16> visitedDirectorySectors;

    while (directoryTrack != 0)
    {
        const quint16 sectorKey = static_cast<quint16>((static_cast<quint16>(directoryTrack) << 8) | directorySectorNumber);
        if (visitedDirectorySectors.contains(sectorKey))
        {
            m_qstrLastError = QStringLiteral("Die Directory-Sektorkette enthält einen Zyklus.");
            return false;
        }

        visitedDirectorySectors.insert(sectorKey);
        if (!readSector(deviceNumber, directoryTrack, directorySectorNumber, directorySector))
            return false;

        for (int entryOffset = 2; entryOffset + 31 < directorySector.size(); entryOffset += 32)
        {
            const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));

            // Ungenutzte Directory-Einträge überspringen.
            if ((fileType & 0x07U) == 0)
                continue;
            // Für LOAD unterstützen wir PRG-Dateien.
            if ((fileType & 0x07U) != 2)
                continue;

            const QByteArray entryName = normalizeDiskFileName(directorySector.mid(entryOffset + 3, 16));
            bool matches = entryName == requestedName;

            // Ein abschließendes * erlaubt die übliche Präfixsuche.
            if (!matches && requestedName.endsWith('*'))
            {
                const QByteArray prefix = requestedName.left(requestedName.size() - 1);
                matches = entryName.startsWith(prefix);
            }

            if (!matches)
                continue;

            fileTrack = static_cast<quint8>(directorySector.at(entryOffset + 1));
            fileSectorNumber = static_cast<quint8>(directorySector.at(entryOffset + 2));
            fileFound = true;
            break;
        }

        if (fileFound)
            break;

        directoryTrack = static_cast<quint8>(directorySector.at(0));
        directorySectorNumber = static_cast<quint8>(directorySector.at(1));
    }

    if (!fileFound)
    {
        m_qstrLastError = QStringLiteral("PRG-Datei nicht gefunden: %1").arg(QString::fromLatin1(requestedName));
        return false;
    }

    QSet<quint16> visitedFileSectors;
    while (fileTrack != 0)
    {
        const quint16 sectorKey = static_cast<quint16>((static_cast<quint16>(fileTrack) << 8) | fileSectorNumber);
        if (visitedFileSectors.contains(sectorKey))
        {
            data.clear();
            m_qstrLastError = QStringLiteral("Die Dateisektorkette enthält einen Zyklus.");
            return false;
        }

        visitedFileSectors.insert(sectorKey);
        QByteArray fileSectorData;
        if (!readSector(deviceNumber, fileTrack, fileSectorNumber, fileSectorData))
        {
            data.clear();
            return false;
        }

        const quint8 nextTrack = static_cast<quint8>(fileSectorData.at(0));
        const quint8 nextSector = static_cast<quint8>(fileSectorData.at(1));
        if (nextTrack != 0)
        {
            // Bei normalen Sektoren sind die ersten zwei Bytes
            // für den Verweis auf den Folgesektor reserviert.
            data.append(fileSectorData.constData() + 2, 254);
        }
        else
        {
            // Im letzten Sektor gibt Byte 1 die Position des
            // letzten verwendeten Bytes einschließlich der
            // beiden Link-Bytes an.
            const int payloadSize = static_cast<int>(nextSector) - 1;
            if (payloadSize < 0 || payloadSize > 254)
            {
                data.clear();
                m_qstrLastError = QStringLiteral("Der letzte Dateisektor enthält eine ungültige Länge.");
                return false;
            }
            data.append(fileSectorData.constData() + 2, payloadSize);
        }

        fileTrack = nextTrack;
        fileSectorNumber = nextSector;
    }

    if (data.size() < 2)
    {
        data.clear();
        m_qstrLastError = QStringLiteral("Die PRG-Datei enthält keine vollständige Ladeadresse.");
        return false;
    }

    return true;
}
