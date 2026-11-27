#pragma once

#include <QByteArray>
#include <QQueue>
#include <QString>
#include <QtGlobal>

#include "IECFoundryDiskConfiguration.h"
#include "IECDeviceEvent.h"


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
    struct FileOpenResult
    {
        bool empty = false;
        quint8 firstTrack = 0;
        quint8 firstSector = 0;
    };
    struct FileDataBlock
    {
        QByteArray data;
        bool isLast = false;
        quint8 nextTrack = 0;
        quint8 nextSector = 0;
    };

    IECFoundryDisk(const IECFoundryDiskConfiguration& configuration);
    ~IECFoundryDisk();

    IECFoundryDisk(const IECFoundryDisk&) = delete;
    IECFoundryDisk& operator=(const IECFoundryDisk&) = delete;

    DiskId diskId() const;
    int slotIndex() const;

    bool enabled() const;
    void setEnabled(bool enabled);
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
    bool openFile(const QByteArray& fileName, FileOpenResult& result);
    bool readFileBlock(quint8 track, quint8 sector, FileDataBlock& block);

    QQueue<IECDriveImageEvent> takeImageEvents();
    QQueue<SectorAccessStatus> takeSectorAccessEvents();

    void setDosStatus(quint8 code, const QByteArray& message, quint8 track = 0, quint8 sector = 0);
    QByteArray takeDosStatus();

private:
    static int sectorsOnTrack(int track);
    static int sectorOffset(int track, int sector);

    static QByteArray normalizeDiskFileName(QByteArray fileName);
    static bool matchesFileName(const QByteArray& pattern, const QByteArray& fileName);

    bool saveImageInternal();

    bool createDirectoryHeader(const QByteArray& bam, QByteArray& header);
    bool createDirectoryEntry(const QByteArray& directorySector, int entryOffset, QByteArray& entry);
    bool createDirectoryFooter(quint16 freeBlocks, QByteArray& footer);

    void recordImageEvent(IECDriveImageEvent::Type type, const QString& fileName = QString(), const QString& errorMessage = QString());
    void recordSectorAccess(SectorAccessOperation operation, quint8 track, quint8 sector, bool success, const QString& errorMessage);
    bool readSectorInternal(quint8 track, quint8 sector, QByteArray& data, bool recordAccess);

private:
    DiskId m_diskId = 0;
    int m_slotIndex = 0;

    bool m_enabled = false;
    bool m_saveCurrentDiskOnExit = false;

    QString m_imageFileName;
    QByteArray m_imageData;
    bool m_imageValid = false;
    bool m_imageModified = false;

    QString m_lastError;
    QQueue<IECDriveImageEvent> m_imageEvents;
    QQueue<SectorAccessStatus> m_sectorAccessEvents;

    quint8 m_dosStatusCode = 0;
    QByteArray m_dosStatusMessage = QByteArrayLiteral("OK");
    quint8 m_dosStatusTrack = 0;
    quint8 m_dosStatusSector = 0;
};
