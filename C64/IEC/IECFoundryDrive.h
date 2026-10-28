#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QtGlobal>

class IECFoundryDisk;


class IECFoundryDrive
{
public:
    explicit IECFoundryDrive();
    virtual ~IECFoundryDrive();

    IECFoundryDrive(const IECFoundryDrive&) = delete;
    IECFoundryDrive& operator=(const IECFoundryDrive&) = delete;

    bool registerDisk(IECFoundryDisk* ptrDisk);
    void unregisterDisk(IECFoundryDisk* ptrDisk);

    int diskCount() const;
    bool hasDisk(quint8 deviceNumber) const;

    bool insertImage(quint8 deviceNumber, const QString& fileName, QString* errorMessage = nullptr);
    bool ejectImage(quint8 deviceNumber, QString* errorMessage = nullptr);
    bool saveImage(quint8 deviceNumber, QString* errorMessage = nullptr);

    bool hasImage(quint8 deviceNumber) const;
    QString imageFileName(quint8 deviceNumber) const;

    bool readSector(quint8 deviceNumber, quint8 track, quint8 sector, QByteArray& data);
    bool writeSector(quint8 deviceNumber, quint8 track, quint8 sector, const QByteArray& data);

private:
    int sectorsOnTrack(int track);
    int sectorOffset(int track, int sector);

private:
    struct DriveState;

    QHash<quint8, DriveState*> m_drives;
};
