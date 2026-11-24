#pragma once

#include <QByteArray>
#include <QQueue>
#include <QString>
#include <QtGlobal>

#include "IECFoundryDiskConfiguration.h"

class IECFoundryDrive;

class IECFoundryDisk
{
public:
    using DiskId = IECFoundryDiskConfiguration::DiskId;

    enum class SectorAccessOperation : quint8
    {
        Read,
        Write
    };

    struct SectorAccessStatus
    {
        SectorAccessOperation operation = SectorAccessOperation::Read;
        quint8 track = 0;
        quint8 sector = 0;
        bool success = false;
        QString errorMessage;
    };

    IECFoundryDisk(IECFoundryDrive* ptrDrive, const IECFoundryDiskConfiguration& configuration);
    ~IECFoundryDisk();

    IECFoundryDisk(const IECFoundryDisk&) = delete;
    IECFoundryDisk& operator=(const IECFoundryDisk&) = delete;

    DiskId diskId() const;
    int slotIndex() const;

    bool enabled() const;
    bool saveCurrentDiskOnExit() const;

    bool setConfiguration(const IECFoundryDiskConfiguration& configuration);

    QString lastError() const;

    bool hasImage() const;
    bool imageValid() const;
    QString imageFileName() const;
    bool imageModified() const;

    bool insertImage(const QString& fileName);
    bool ejectImage();
    bool saveImage();

    bool readSector(quint8 track, quint8 sector, QByteArray& data);
    bool writeSector(quint8 track, quint8 sector, const QByteArray& data);

    bool createDirectoryProgram(QByteArray& program);
    bool readFile(const QByteArray& fileName, QByteArray& data);

    QQueue<SectorAccessStatus> takeSectorAccessEvents();

private:
    static int sectorsOnTrack(int track);
    static int sectorOffset(int track, int sector);

    static QByteArray normalizeDiskFileName(QByteArray fileName);

    bool saveImageInternal();

    bool createDirectoryHeader(const QByteArray& bam, QByteArray& header);
    bool createDirectoryEntry(const QByteArray& directorySector, int entryOffset, QByteArray& entry);
    bool createDirectoryFooter(quint16 freeBlocks, QByteArray& footer);

    void debugDirectorySector(const QByteArray& directorySector, quint8 track, quint8 sector);

    void recordSectorAccess(SectorAccessOperation operation, quint8 track, quint8 sector, bool success, const QString& errorMessage);

private:
    IECFoundryDrive* m_ptrDrive = nullptr; // non-owning

    DiskId m_diskId = 0;
    int m_slotIndex = 0;

    bool m_enabled = false;
    bool m_saveCurrentDiskOnExit = false;

    QString m_imageFileName;
    QByteArray m_imageData;
    bool m_imageValid = false;
    bool m_imageModified = false;

    QString m_lastError;
    QQueue<SectorAccessStatus> m_sectorAccessEvents;
};
