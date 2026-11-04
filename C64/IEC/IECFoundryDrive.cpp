#include "IECFoundryDrive.h"

#include <QFile>
#include <QSaveFile>
#include <QSet>

#include "IECFoundryDisk.h"

#include <QDebug>


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

    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
    {
        m_qstrLastError = QStringLiteral("Das Laufwerk ist nicht vorhanden.");
        recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Read, track, sector, false, m_qstrLastError);
        return false;
    }

    if (ptrState->imageFileName.isEmpty() || ptrState->imageData.isNull())
    {
        m_qstrLastError = QStringLiteral("Es ist kein Disketten-Image geladen.");
        recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Read, track, sector, false, m_qstrLastError);
        return false;
    }

    const int offset = sectorOffset(track, sector);

    if (offset < 0)
    {
        m_qstrLastError = QStringLiteral("Ungültiger Track oder Sektor.");
        recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Read, track, sector, false, m_qstrLastError);
        return false;
    }

    constexpr int SECTOR_SIZE = 256;

    if (offset + SECTOR_SIZE > ptrState->imageData.size())
    {
        m_qstrLastError = QStringLiteral("Der Sektor liegt außerhalb des Disketten-Images.");
        recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Read, track, sector, false, m_qstrLastError);
        return false;
    }

    data = ptrState->imageData.mid(offset, SECTOR_SIZE);

    if (data.size() != SECTOR_SIZE)
    {
        data.clear();
        m_qstrLastError = QStringLiteral("Der Sektor konnte nicht vollständig gelesen werden.");
        recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Read, track, sector, false, m_qstrLastError);
        return false;
    }

    recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Read, track, sector, true, QString());

    return true;
}

bool IECFoundryDrive::writeSector(const quint8 deviceNumber, const quint8 track, const quint8 sector, const QByteArray& data)
{
    Q_UNUSED(data);

    m_qstrLastError.clear();

    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr || ptrState->ptrDisk == nullptr)
    {
        m_qstrLastError = QStringLiteral("Das Laufwerk ist nicht vorhanden.");
        recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Write, track, sector, false, m_qstrLastError);
        return false;
    }

    m_qstrLastError = QStringLiteral("Das Schreiben von Sektoren ist noch nicht implementiert.");

    recordSectorAccess(ptrState, deviceNumber, SectorAccessOperation::Write, track, sector, false, m_qstrLastError);

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

    QByteArray header;

    if (!createDirectoryHeader(bam, header))
    {
        m_qstrLastError = QStringLiteral("Der Directory-Header konnte nicht erzeugt werden.");
        return false;
    }

    program.append(header);

    quint8 track = 18;
    quint8 sector = 1;
    QSet<quint16> visitedSectors;

    while (track != 0)
    {
        const quint16 sectorKey =
            (static_cast<quint16>(track) << 8) | sector;

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

        debugDirectorySector(directorySector, track, sector);

        for (int entryOffset = 2; entryOffset + 29 < directorySector.size(); entryOffset += 32)
        {
            const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));

            if (fileType == 0)
                continue;

            QByteArray entry;

            if (!createDirectoryEntry(directorySector, entryOffset, entry))
            {
                m_qstrLastError = QStringLiteral("Ein Directory-Eintrag konnte nicht erzeugt werden.");
                program.clear();
                return false;
            }

            program.append(entry);
        }

        track = static_cast<quint8>(directorySector.at(0));
        sector = static_cast<quint8>(directorySector.at(1));
    }

    quint16 freeBlocks = 0;

    for (int currentTrack = 1; currentTrack <= 35; ++currentTrack)
    {
        if (currentTrack == 18)
            continue;

        const int bamOffset = 4 + ((currentTrack - 1) * 4);
        freeBlocks += static_cast<quint8>(bam.at(bamOffset));
    }

    QByteArray footer;

    if (!createDirectoryFooter(freeBlocks, footer))
    {
        m_qstrLastError = QStringLiteral("Der Directory-Footer konnte nicht erzeugt werden.");
        program.clear();
        return false;
    }

    program.append(footer);

    qDebug() << "IEC directory program:"
             << "size:" << program.size()
             << "load address:" << Qt::hex
             << static_cast<quint8>(program.at(0))
             << static_cast<quint8>(program.at(1));

    qDebug().noquote() << "IEC directory program:" << program.toHex(' ');

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

        for (int entryOffset = 2; entryOffset + 29 < directorySector.size(); entryOffset += 32)
        {
            const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));

            // Ungenutzte Directory-Einträge überspringen.
            if (fileType == 0)
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

void IECFoundryDrive::debugDirectorySector(const QByteArray& directorySector, const quint8 track, const quint8 sector)
{
    qDebug() << "IEC directory sector:"
             << "track:" << track
             << "sector:" << sector
             << "size:" << directorySector.size();

    qDebug().noquote() << directorySector.toHex(' ');

    if (directorySector.size() != 256)
        return;

    for (int entryOffset = 2; entryOffset + 29 < directorySector.size(); entryOffset += 32)
    {
        const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));

        if ((fileType & 0x07U) == 0)
            continue;

        const QByteArray fileName = directorySector.mid(entryOffset + 3, 16);

        const quint8 blockLow = static_cast<quint8>(directorySector.at(entryOffset + 28));
        const quint8 blockHigh = static_cast<quint8>(directorySector.at(entryOffset + 29));

        const quint16 blocks = static_cast<quint16>(
            static_cast<quint16>(blockLow) |
            (static_cast<quint16>(blockHigh) << 8));

        qDebug() << "IEC directory entry:"
                 << "offset:" << entryOffset
                 << "type:" << Qt::hex << fileType
                 << "name:" << fileName.toHex(' ')
                 << "blocks:" << Qt::dec << blocks
                 << "block bytes:" << Qt::hex << blockLow << blockHigh;
    }
}

bool IECFoundryDrive::createDirectoryHeader(const QByteArray& bam, QByteArray& header)
{
    header.clear();

    if (bam.size() != 256)
        return false;

    header = QByteArray(32, ' ');

    // BASIC-Ladeadresse $0401.
    header[0] = static_cast<char>(0x01);
    header[1] = static_cast<char>(0x04);

    // Zeiger auf den nächsten BASIC-Datensatz.
    header[2] = static_cast<char>(0x01);
    header[3] = static_cast<char>(0x01);

    // Verzeichnis-Header: Zeilennummer 0.
    header[4] = static_cast<char>(0x00);
    header[5] = static_cast<char>(0x00);

    // Reverse on, öffnendes Anführungszeichen.
    header[6] = static_cast<char>(0x12);
    header[7] = '"';

    // Diskettenname: exakt 16 Zeichen.
    for (int index = 0; index < 16; ++index)
    {
        quint8 value = static_cast<quint8>(bam.at(144 + index));

        if (value == 0xA0)
            value = 0x20;

        header[8 + index] = static_cast<char>(value & 0x7F);
    }

    // Schließendes Anführungszeichen und Leerzeichen.
    header[24] = '"';
    header[25] = ' ';

    // Disketten-ID und DOS-Typ einschließlich Trennzeichen.
    for (int index = 0; index < 5; ++index)
    {
        quint8 value = static_cast<quint8>(bam.at(162 + index));

        if (value == 0xA0)
            value = 0x20;

        header[26 + index] = static_cast<char>(value & 0x7F);
    }

    // Ende der BASIC-Zeile.
    header[31] = static_cast<char>(0x00);

    return true;
}

bool IECFoundryDrive::createDirectoryEntry(const QByteArray& directorySector, int entryOffset, QByteArray& entry)
{
    if (entryOffset < 2 || entryOffset + 29 >= directorySector.size())
    {
        return false;
    }

    const quint8 fileTypeFlags = static_cast<quint8>(directorySector.at(entryOffset));
    const quint16 blocks = static_cast<quint8>(directorySector.at(entryOffset + 28))
                           | (static_cast<quint16>(static_cast<quint8>(directorySector.at(entryOffset + 29))) << 8);

    const QByteArray fileName = directorySector.mid(entryOffset + 3, 16);

    int nameLength = 16;

    for (int index = 0; index < 16; ++index)
    {
        const quint8 character = static_cast<quint8>(fileName.at(index));

        if (character == 0x22 || character == 0x00)
        {
            nameLength = index;
            break;
        }
    }

    entry = QByteArray(32, ' ');
    entry[0] = 0x01;
    entry[1] = 0x01;
    entry[2] = static_cast<char>(blocks & 0xFF);
    entry[3] = static_cast<char>((blocks >> 8) & 0xFF);

    int position = 4;
    // Festes Leerzeichen nach der Blockzahl.
    ++position;

    if (blocks < 100)
    {
        ++position;
    }
    if (blocks < 10)
    {
        ++position;
    }

    entry[position++] = '"';

    const int nameStart = position;

    for (int index = 0; index < 16; ++index)
    {
        quint8 character = static_cast<quint8>(fileName.at(index));

        if (index < nameLength)
        {
            entry[nameStart + index] = static_cast<char>(character);
        }
        else
        {
            if (character == 0x00)
                character = 0x20;
            else
                character &= 0x7F;

            entry[nameStart + index] = static_cast<char>(character);
        }
    }

    entry[nameStart + nameLength] = '"';

    const int typePosition = nameStart + 17;

    if ((fileTypeFlags & 0x80) == 0)
    {
        entry[typePosition] = '*';
    }

    const quint8 type = fileTypeFlags & 0x07;
    const char* typeName = "???";

    switch (type)
    {
    case 0: typeName = "DEL"; break;
    case 1: typeName = "SEQ"; break;
    case 2: typeName = "PRG"; break;
    case 3: typeName = "USR"; break;
    case 4: typeName = "REL"; break;
    default: break;
    }

    entry[typePosition + 1] = typeName[0];
    entry[typePosition + 2] = typeName[1];
    entry[typePosition + 3] = typeName[2];

    if ((fileTypeFlags & 0x40) != 0)
    {
        entry[typePosition + 4] = '<';
    }

    entry[31] = '\0';
    return true;
}

bool IECFoundryDrive::createDirectoryFooter(const quint16 freeBlocks, QByteArray& footer)
{
    footer = QByteArray(32, ' ');

    footer[0] = static_cast<char>(0x01);
    footer[1] = static_cast<char>(0x01);

    footer[2] = static_cast<char>(freeBlocks & 0xFFU);
    footer[3] = static_cast<char>((freeBlocks >> 8) & 0xFFU);

    const QByteArray text = QByteArrayLiteral("BLOCKS FREE.");
    footer.replace(4, text.size(), text);

    // SD2IEC-Footer endet mit drei Nullbytes.
    footer[29] = static_cast<char>(0x00);
    footer[30] = static_cast<char>(0x00);
    footer[31] = static_cast<char>(0x00);

    return true;
}

void IECFoundryDrive::recordSectorAccess(DriveState* ptrState, const quint8 deviceNumber, const SectorAccessOperation operation, const quint8 track, const quint8 sector, const bool success, const QString& errorMessage)
{
    if (ptrState == nullptr)
        return;

    SectorAccessStatus status;
    status.deviceNumber = deviceNumber;
    status.operation = operation;
    status.track = track;
    status.sector = sector;
    status.success = success;
    status.errorMessage = errorMessage;

    ptrState->sectorAccessEvents.append(status);
}
QVector<IECFoundryDrive::SectorAccessStatus> IECFoundryDrive::takeSectorAccessEvents(const quint8 deviceNumber)
{
    DriveState* ptrState = m_drives.value(deviceNumber, nullptr);

    if (ptrState == nullptr)
        return {};

    QQueue<SectorAccessStatus> events;
    events.swap(ptrState->sectorAccessEvents);

    return events;
}
