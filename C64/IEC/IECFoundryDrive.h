#pragma once

#include <QByteArray>
#include <QHash>
#include <QQueue>
#include <QString>
#include <QtGlobal>
#include <QMetaType>


class IECFoundryDisk;

class IECFoundryDrive
{
public:
    enum class SectorAccessOperation : quint8
    {
        Read,
        Write
    };
    struct SectorAccessStatus
    {
        quint8 deviceNumber = 0;
        SectorAccessOperation operation = SectorAccessOperation::Read;
        quint8 track = 0;
        quint8 sector = 0;
        bool success = false;
        QString errorMessage;
    };
    struct DriveState
    {
        IECFoundryDisk* ptrDisk = nullptr;
        QString imageFileName;
        QByteArray imageData;
        bool imageModified = false;
        QQueue<SectorAccessStatus> sectorAccessEvents;
    };

    explicit IECFoundryDrive();
    virtual ~IECFoundryDrive();

    IECFoundryDrive(const IECFoundryDrive&) = delete;
    IECFoundryDrive& operator=(const IECFoundryDrive&) = delete;

    QString lastError() const                       { return m_qstrLastError; }
    QQueue<SectorAccessStatus> takeSectorAccessEvents(quint8 deviceNumber);

    bool registerDisk(IECFoundryDisk* ptrDisk);
    void unregisterDisk(IECFoundryDisk* ptrDisk);

    int diskCount() const;
    bool hasDisk(quint8 deviceNumber) const;

    bool insertImage(quint8 deviceNumber, const QString& fileName);
    bool ejectImage(quint8 deviceNumber);
    bool saveImage(quint8 deviceNumber);

    bool hasImage(quint8 deviceNumber) const;
    QString imageFileName(quint8 deviceNumber) const;

    bool readSector(quint8 deviceNumber, quint8 track, quint8 sector, QByteArray& data);
    bool writeSector(quint8 deviceNumber, quint8 track, quint8 sector, const QByteArray& data);

    bool createDirectoryProgram(quint8 deviceNumber, QByteArray& program);
    bool readFile(quint8 deviceNumber, const QByteArray& fileName, QByteArray& data);

private:
    int sectorsOnTrack(int track);
    int sectorOffset(int track, int sector);

    bool createDirectoryHeader(const QByteArray& bam, QByteArray& header);
    bool createDirectoryEntry(const QByteArray& directorySector, int entryOffset, QByteArray& entry);
    bool createDirectoryFooter(quint16 freeBlocks, QByteArray& footer);

    QByteArray normalizeDiskFileName(QByteArray fileName);
    bool saveImageInternal(quint8 deviceNumber);
    void debugDirectorySector(const QByteArray& directorySector, quint8 track, quint8 sector);

    void recordSectorAccess(DriveState* ptrState, quint8 deviceNumber, SectorAccessOperation operation, quint8 track, quint8 sector, bool success, const QString& errorMessage);

private:
    QHash<quint8, DriveState*> m_drives;
    QString m_qstrLastError;
};

Q_DECLARE_METATYPE(IECFoundryDrive::SectorAccessStatus)