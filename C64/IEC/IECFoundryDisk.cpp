#include "IECFoundryDisk.h"

#include "IECFoundryDrive.h"

#include <QDebug>
#include <QFile>
#include <QSaveFile>
#include <QSet>


IECFoundryDisk::IECFoundryDisk(IECFoundryDrive* ptrDrive, const IECFoundryDiskConfiguration& configuration)
    : m_ptrDrive(ptrDrive)
{
    setConfiguration(configuration);
}

IECFoundryDisk::~IECFoundryDisk()
{
}


IECFoundryDisk::DiskId IECFoundryDisk::diskId() const
{
    return m_diskId;
}

int IECFoundryDisk::slotIndex() const
{
    return m_slotIndex;
}

bool IECFoundryDisk::enabled() const
{
    return m_enabled;
}
void IECFoundryDisk::setEnabled(const bool enabled)
{
    m_enabled = enabled;
}

bool IECFoundryDisk::saveCurrentDiskOnExit() const
{
    return m_saveCurrentDiskOnExit;
}

bool IECFoundryDisk::setConfiguration(const IECFoundryDiskConfiguration& configuration)
{
    m_diskId = configuration.diskId();
    m_slotIndex = configuration.slotIndex();
    m_enabled = configuration.enabled();
    m_saveCurrentDiskOnExit = configuration.saveCurrentDiskOnExit();

    if (configuration.imageFileName() != m_imageFileName)
        insertImage(configuration.imageFileName());

    return true;
}

QString IECFoundryDisk::lastError() const
{
    return m_lastError;
}


bool IECFoundryDisk::hasImage() const
{
    return !m_imageFileName.isEmpty();
}
bool IECFoundryDisk::imageValid() const
{
    return m_imageValid;
}
QString IECFoundryDisk::imageFileName() const
{
    return m_imageFileName;
}
bool IECFoundryDisk::imageModified() const
{
    return m_imageModified;
}


bool IECFoundryDisk::insertImage(const QString& fileName)
{
    m_lastError.clear();

    if (fileName.isEmpty())
        return ejectImage();

    // Das bisherige Image darf das Ersetzen nicht verhindern.
    if (m_imageModified && !saveImageInternal())
    {
        recordImageEvent(IECDriveImageEvent::Type::Error,
                         m_imageFileName,
                         m_lastError);
    }

    m_imageFileName = fileName;
    m_imageData.clear();
    m_imageValid = false;
    m_imageModified = false;
    m_lastError.clear();

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        m_lastError = QStringLiteral("Image konnte nicht geöffnet werden: %1\n%2")
                          .arg(fileName, file.errorString());

        recordImageEvent(IECDriveImageEvent::Type::ImageInvalid,
                         fileName,
                         m_lastError);

        recordImageEvent(IECDriveImageEvent::Type::Error,
                         fileName,
                         m_lastError);

        return false;
    }

    const QByteArray imageData = file.readAll();

    if (file.error() != QFileDevice::NoError)
    {
        m_lastError = QStringLiteral("Fehler beim Lesen des Images: %1\n%2")
        .arg(fileName, file.errorString());

        recordImageEvent(IECDriveImageEvent::Type::ImageInvalid,
                         fileName,
                         m_lastError);

        recordImageEvent(IECDriveImageEvent::Type::Error,
                         fileName,
                         m_lastError);

        return false;
    }

    m_imageData = imageData;
    m_imageValid = true;

    recordImageEvent(IECDriveImageEvent::Type::ImageInserted, fileName);

    return true;
}
bool IECFoundryDisk::ejectImage()
{
    m_lastError.clear();

    if (m_imageModified && !saveImageInternal())
    {
        recordImageEvent(IECDriveImageEvent::Type::Error,
                         m_imageFileName,
                         m_lastError);

        return false;
    }

    const QString previousFileName = m_imageFileName;

    m_imageFileName.clear();
    m_imageData.clear();
    m_imageValid = false;
    m_imageModified = false;

    recordImageEvent(IECDriveImageEvent::Type::ImageEjected,
                     previousFileName);

    return true;
}
bool IECFoundryDisk::saveImage()
{
    m_lastError.clear();

    if (saveImageInternal())
        return true;

    recordImageEvent(IECDriveImageEvent::Type::Error,
                     m_imageFileName,
                     m_lastError);

    return false;
}

bool IECFoundryDisk::saveImageInternal()
{
    if (!imageValid())
    {
        m_lastError = QStringLiteral("Es ist kein Image zum Speichern geladen.");
        return false;
    }

    QSaveFile file(m_imageFileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        m_lastError = QStringLiteral("Image konnte nicht zum Schreiben geöffnet werden: %1\n%2").arg(m_imageFileName, file.errorString());
        return false;
    }

    if (file.write(m_imageData) != m_imageData.size())
    {
        m_lastError = QStringLiteral("Fehler beim Schreiben des Images: %1\n%2").arg(m_imageFileName, file.errorString());
        file.cancelWriting();
        return false;
    }

    if (!file.commit())
    {
        m_lastError = QStringLiteral("Image konnte nicht gespeichert werden: %1\n%2").arg(m_imageFileName, file.errorString());
        return false;
    }

    m_imageModified = false;
    return true;
}


int IECFoundryDisk::sectorsOnTrack(const int track)
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

int IECFoundryDisk::sectorOffset(const int track, const int sector)
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

QByteArray IECFoundryDisk::normalizeDiskFileName(QByteArray fileName)
{
    while (!fileName.isEmpty() && (static_cast<quint8>(fileName.back()) == 0xA0 || fileName.back() == ' '))
        fileName.chop(1);

    return fileName.toUpper();
}


bool IECFoundryDisk::readSector(const quint8 track, const quint8 sector, QByteArray& data)
{
    return readSectorInternal(track, sector, data, true);
}


bool IECFoundryDisk::readSectorInternal(
    const quint8 track,
    const quint8 sector,
    QByteArray& data,
    const bool recordAccess)
{
    m_lastError.clear();
    data.clear();

    if (!imageValid())
    {
        m_lastError = QStringLiteral("Es ist kein Disketten-Image geladen.");

        if (recordAccess)
            recordSectorAccess(SectorAccessOperation::Read, track, sector, false, m_lastError);

        return false;
    }

    const int offset = sectorOffset(track, sector);

    if (offset < 0)
    {
        m_lastError = QStringLiteral("Ungültiger Track oder Sektor.");

        if (recordAccess)
            recordSectorAccess(SectorAccessOperation::Read, track, sector, false, m_lastError);

        return false;
    }

    constexpr int SectorSize = 256;

    if (offset + SectorSize > m_imageData.size())
    {
        m_lastError = QStringLiteral("Der Sektor liegt außerhalb des Disketten-Images.");

        if (recordAccess)
            recordSectorAccess(SectorAccessOperation::Read, track, sector, false, m_lastError);

        return false;
    }

    data = m_imageData.mid(offset, SectorSize);

    if (data.size() != SectorSize)
    {
        data.clear();
        m_lastError = QStringLiteral("Der Sektor konnte nicht vollständig gelesen werden.");

        if (recordAccess)
            recordSectorAccess(SectorAccessOperation::Read, track, sector, false, m_lastError);

        return false;
    }

    if (recordAccess)
        recordSectorAccess(SectorAccessOperation::Read, track, sector, true, QString());

    return true;
}

bool IECFoundryDisk::writeSector(const quint8 track, const quint8 sector, const QByteArray& data)
{
    Q_UNUSED(data);

    m_lastError.clear();

    if (!imageValid())
    {
        m_lastError = QStringLiteral("Es ist kein Disketten-Image geladen.");
        recordSectorAccess(SectorAccessOperation::Write, track, sector, false, m_lastError);
        return false;
    }

    m_lastError = QStringLiteral("Das Schreiben von Sektoren ist noch nicht implementiert.");
    recordSectorAccess(SectorAccessOperation::Write, track, sector, false, m_lastError);

    return false;
}


bool IECFoundryDisk::createDirectoryProgram(QByteArray& program)
{
    m_lastError.clear();
    program.clear();

    if (!imageValid())
    {
        m_lastError = QStringLiteral("Kein Disketten-Image geladen.");
        return false;
    }

    QByteArray bam;

    if (!readSector(18, 0, bam) || bam.size() != 256)
        return false;

    QByteArray header;

    if (!createDirectoryHeader(bam, header))
    {
        m_lastError = QStringLiteral("Der Directory-Header konnte nicht erzeugt werden.");
        return false;
    }

    program.append(header);

    quint8 track = 18;
    quint8 sector = 1;
    QSet<quint16> visitedSectors;

    while (track != 0)
    {
        const quint16 sectorKey = (static_cast<quint16>(track) << 8) | sector;

        if (visitedSectors.contains(sectorKey))
        {
            m_lastError = QStringLiteral("Die Directory-Sektorkette enthält einen Zyklus.");
            program.clear();
            return false;
        }

        visitedSectors.insert(sectorKey);

        QByteArray directorySector;

        if (!readSector(track, sector, directorySector) || directorySector.size() != 256)
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
                m_lastError = QStringLiteral("Ein Directory-Eintrag konnte nicht erzeugt werden.");
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
        m_lastError = QStringLiteral("Der Directory-Footer konnte nicht erzeugt werden.");
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


bool IECFoundryDisk::readFile(const QByteArray& fileName, QByteArray& data)
{
    m_lastError.clear();
    data.clear();

    if (!imageValid())
    {
        m_lastError = QStringLiteral("Kein Disketten-Image geladen.");
        return false;
    }

    const QByteArray requestedName = normalizeDiskFileName(fileName);

    if (requestedName.isEmpty())
    {
        m_lastError = QStringLiteral("Es wurde kein Dateiname angegeben.");
        return false;
    }

    QByteArray directorySector;

    if (!readSector(18, 1, directorySector))
        return false;

    quint8 directoryTrack = 18;
    quint8 directorySectorNumber = 1;
    quint8 fileTrack = 0;
    quint8 fileSectorNumber = 0;
    bool fileFound = false;
    QSet<quint16> visitedDirectorySectors;

    while (directoryTrack != 0)
    {
        const quint16 sectorKey = (static_cast<quint16>(directoryTrack) << 8) | directorySectorNumber;

        if (visitedDirectorySectors.contains(sectorKey))
        {
            m_lastError = QStringLiteral("Die Directory-Sektorkette enthält einen Zyklus.");
            return false;
        }

        visitedDirectorySectors.insert(sectorKey);

        if (!readSector(directoryTrack, directorySectorNumber, directorySector))
            return false;

        for (int entryOffset = 2; entryOffset + 29 < directorySector.size(); entryOffset += 32)
        {
            const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));

            if (fileType == 0)
                continue;

            if ((fileType & 0x07U) != 2)
                continue;

            const QByteArray entryName = normalizeDiskFileName(directorySector.mid(entryOffset + 3, 16));
            bool matches = entryName == requestedName;

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
        m_lastError = QStringLiteral("PRG-Datei nicht gefunden: %1").arg(QString::fromLatin1(requestedName));
        return false;
    }

    QSet<quint16> visitedFileSectors;

    while (fileTrack != 0)
    {
        const quint16 sectorKey = (static_cast<quint16>(fileTrack) << 8) | fileSectorNumber;

        if (visitedFileSectors.contains(sectorKey))
        {
            data.clear();
            m_lastError = QStringLiteral("Die Dateisektorkette enthält einen Zyklus.");
            return false;
        }

        visitedFileSectors.insert(sectorKey);

        QByteArray fileSectorData;

        if (!readSector(fileTrack, fileSectorNumber, fileSectorData))
        {
            data.clear();
            return false;
        }

        const quint8 nextTrack = static_cast<quint8>(fileSectorData.at(0));
        const quint8 nextSector = static_cast<quint8>(fileSectorData.at(1));

        if (nextTrack != 0)
        {
            data.append(fileSectorData.constData() + 2, 254);
        }
        else
        {
            const int payloadSize = static_cast<int>(nextSector) - 1;

            if (payloadSize < 0 || payloadSize > 254)
            {
                data.clear();
                m_lastError = QStringLiteral("Der letzte Dateisektor enthält eine ungültige Länge.");
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
        m_lastError = QStringLiteral("Die PRG-Datei enthält keine vollständige Ladeadresse.");
        return false;
    }

    return true;
}


bool IECFoundryDisk::createDirectoryHeader(const QByteArray& bam, QByteArray& header)
{
    header.clear();

    if (bam.size() != 256)
        return false;

    header = QByteArray(32, ' ');

    header[0] = static_cast<char>(0x01);
    header[1] = static_cast<char>(0x04);

    header[2] = static_cast<char>(0x01);
    header[3] = static_cast<char>(0x01);

    header[4] = static_cast<char>(0x00);
    header[5] = static_cast<char>(0x00);

    header[6] = static_cast<char>(0x12);
    header[7] = '"';

    for (int index = 0; index < 16; ++index)
    {
        quint8 value = static_cast<quint8>(bam.at(144 + index));

        if (value == 0xA0)
            value = 0x20;

        header[8 + index] = static_cast<char>(value & 0x7F);
    }

    header[24] = '"';
    header[25] = ' ';

    for (int index = 0; index < 5; ++index)
    {
        quint8 value = static_cast<quint8>(bam.at(162 + index));

        if (value == 0xA0)
            value = 0x20;

        header[26 + index] = static_cast<char>(value & 0x7F);
    }

    header[31] = static_cast<char>(0x00);

    return true;
}

bool IECFoundryDisk::createDirectoryEntry(const QByteArray& directorySector, const int entryOffset, QByteArray& entry)
{
    if (entryOffset < 2 || entryOffset + 29 >= directorySector.size())
        return false;

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

    int position = 5;

    if (blocks < 100)
        ++position;

    if (blocks < 10)
        ++position;

    entry[position++] = '"';

    const int nameStart = position;

    for (int index = 0; index < 16; ++index)
    {
        quint8 character = static_cast<quint8>(fileName.at(index));

        if (index < nameLength)
            entry[nameStart + index] = static_cast<char>(character);
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
        entry[typePosition] = '*';

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
        entry[typePosition + 4] = '<';

    entry[31] = '\0';

    return true;
}

bool IECFoundryDisk::createDirectoryFooter(const quint16 freeBlocks, QByteArray& footer)
{
    footer = QByteArray(32, ' ');

    footer[0] = static_cast<char>(0x01);
    footer[1] = static_cast<char>(0x01);

    footer[2] = static_cast<char>(freeBlocks & 0xFFU);
    footer[3] = static_cast<char>((freeBlocks >> 8) & 0xFFU);

    const QByteArray text = QByteArrayLiteral("BLOCKS FREE.");
    footer.replace(4, text.size(), text);

    footer[29] = static_cast<char>(0x00);
    footer[30] = static_cast<char>(0x00);
    footer[31] = static_cast<char>(0x00);

    return true;
}


void IECFoundryDisk::debugDirectorySector(const QByteArray& directorySector, const quint8 track, const quint8 sector)
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

        const quint16 blocks = static_cast<quint16>(blockLow) | (static_cast<quint16>(blockHigh) << 8);

        qDebug() << "IEC directory entry:"
                 << "offset:" << entryOffset
                 << "type:" << Qt::hex << fileType
                 << "name:" << fileName.toHex(' ')
                 << "blocks:" << Qt::dec << blocks
                 << "block bytes:" << Qt::hex << blockLow << blockHigh;
    }
}


void IECFoundryDisk::recordImageEvent(
    const IECDriveImageEvent::Type type,
    const QString& fileName,
    const QString& errorMessage)
{
    IECDriveImageEvent event;
    event.type = type;
    event.fileName = fileName;
    event.errorMessage = errorMessage;

    m_imageEvents.enqueue(event);
}
void IECFoundryDisk::recordSectorAccess(const SectorAccessOperation operation, const quint8 track, const quint8 sector, const bool success, const QString& errorMessage)
{
    SectorAccessStatus status;
    status.operation = operation;
    status.track = track;
    status.sector = sector;
    status.success = success;
    status.errorMessage = errorMessage;

    m_sectorAccessEvents.append(status);
}

QQueue<IECDriveImageEvent> IECFoundryDisk::takeImageEvents()
{
    QQueue<IECDriveImageEvent> events;
    events.swap(m_imageEvents);

    return events;
}
QQueue<IECFoundryDisk::SectorAccessStatus> IECFoundryDisk::takeSectorAccessEvents()
{
    QQueue<SectorAccessStatus> events;
    events.swap(m_sectorAccessEvents);
    return events;
}

bool IECFoundryDisk::openFile(const QByteArray& fileName, FileOpenResult& result)
{
    m_lastError.clear();
    result = FileOpenResult();

    if (!imageValid())
    {
        m_lastError = QStringLiteral("Kein Disketten-Image geladen.");
        return false;
    }

    const QByteArray requestedName = normalizeDiskFileName(fileName);

    if (requestedName.isEmpty())
    {
        m_lastError = QStringLiteral("Es wurde kein Dateiname angegeben.");
        return false;
    }

    quint8 directoryTrack = 18;
    quint8 directorySectorNumber = 1;
    quint8 fileTrack = 0;
    quint8 fileSectorNumber = 0;
    bool fileFound = false;

    QSet<quint16> visitedDirectorySectors;

    while (directoryTrack != 0)
    {
        const quint16 sectorKey =
            (static_cast<quint16>(directoryTrack) << 8) | directorySectorNumber;

        if (visitedDirectorySectors.contains(sectorKey))
        {
            m_lastError = QStringLiteral("Die Directory-Sektorkette enthält einen Zyklus.");
            return false;
        }

        visitedDirectorySectors.insert(sectorKey);

        QByteArray directorySector;

        if (!readSectorInternal(directoryTrack, directorySectorNumber, directorySector, false))
            return false;

        for (int entryOffset = 2; entryOffset + 29 < directorySector.size(); entryOffset += 32)
        {
            const quint8 fileType = static_cast<quint8>(directorySector.at(entryOffset));

            if ((fileType & 0x07U) != 2)
                continue;

            const QByteArray entryName =
                normalizeDiskFileName(directorySector.mid(entryOffset + 3, 16));

            bool matches = entryName == requestedName;

            if (!matches && requestedName.endsWith('*'))
            {
                const QByteArray prefix =
                    requestedName.left(requestedName.size() - 1);

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
        m_lastError = QStringLiteral("PRG-Datei nicht gefunden: %1")
        .arg(QString::fromLatin1(requestedName));
        return false;
    }

    if (fileTrack == 0)
    {
        if (fileSectorNumber != 0)
        {
            m_lastError = QStringLiteral("Der Directory-Eintrag einer leeren Datei ist ungültig.");
            return false;
        }

        result.empty = true;
        return true;
    }

    const quint8 firstTrack = fileTrack;
    const quint8 firstSector = fileSectorNumber;
    QSet<quint16> visitedFileSectors;
    quint32 fileSize = 0;

    while (fileTrack != 0)
    {
        const quint16 sectorKey =
            (static_cast<quint16>(fileTrack) << 8) | fileSectorNumber;

        if (visitedFileSectors.contains(sectorKey))
        {
            m_lastError = QStringLiteral("Die Dateisektorkette enthält einen Zyklus.");
            return false;
        }

        visitedFileSectors.insert(sectorKey);

        QByteArray fileSectorData;

        if (!readSectorInternal(fileTrack, fileSectorNumber, fileSectorData, false))
            return false;

        const quint8 nextTrack = static_cast<quint8>(fileSectorData.at(0));
        const quint8 nextSector = static_cast<quint8>(fileSectorData.at(1));

        if (nextTrack != 0)
        {
            if (sectorOffset(nextTrack, nextSector) < 0)
            {
                m_lastError = QStringLiteral("Die Dateisektorkette enthält einen ungültigen Folge-Sektor.");
                return false;
            }

            fileSize += 254;
        }
        else
        {
            const int payloadSize = static_cast<int>(nextSector) - 1;

            if (payloadSize < 0 || payloadSize > 254)
            {
                m_lastError = QStringLiteral("Der letzte Dateisektor enthält eine ungültige Länge.");
                return false;
            }

            fileSize += static_cast<quint32>(payloadSize);
        }

        fileTrack = nextTrack;
        fileSectorNumber = nextSector;
    }

    if (fileSize < 2)
    {
        m_lastError = QStringLiteral("Die PRG-Datei enthält keine vollständige Ladeadresse.");
        return false;
    }

    result.firstTrack = firstTrack;
    result.firstSector = firstSector;

    // Die ursprünglichen Startkoordinaten stammen aus dem Directory-Eintrag.
    // Sie werden weiter unten separat gesetzt.
    return true;
}

bool IECFoundryDisk::readFileBlock(
    const quint8 track,
    const quint8 sector,
    FileDataBlock& block)
{
    m_lastError.clear();
    block = FileDataBlock();

    QByteArray sectorData;

    if (!readSector(track, sector, sectorData))
        return false;

    const quint8 nextTrack = static_cast<quint8>(sectorData.at(0));
    const quint8 nextSector = static_cast<quint8>(sectorData.at(1));

    if (nextTrack != 0)
    {
        if (sectorOffset(nextTrack, nextSector) < 0)
        {
            m_lastError = QStringLiteral("Die Dateisektorkette enthält einen ungültigen Folge-Sektor.");
            return false;
        }

        block.data = sectorData.mid(2, 254);
        block.isLast = false;
        block.nextTrack = nextTrack;
        block.nextSector = nextSector;
    }
    else
    {
        const int payloadSize = static_cast<int>(nextSector) - 1;

        if (payloadSize < 0 || payloadSize > 254)
        {
            m_lastError = QStringLiteral("Der letzte Dateisektor enthält eine ungültige Länge.");
            return false;
        }

        block.data = sectorData.mid(2, payloadSize);
        block.isLast = true;
    }

    return true;
}
