#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>


class IECFoundryDrive
{
public:
    enum class State
    {
        Empty,
        Ready,
        Modified,
        Error
    };

    enum class Error
    {
        None,
        FileNotFound,
        FileOpenFailed,
        InvalidImage,
        UnsupportedImage,
        InvalidTrack,
        InvalidSector,
        WriteFailed
    };

    explicit IECFoundryDrive();
    virtual ~IECFoundryDrive();

    IECFoundryDrive(const IECFoundryDrive&) = delete;
    IECFoundryDrive& operator=(const IECFoundryDrive&) = delete;

    State state() const;
    Error error() const;

    bool hasImage() const;
    bool isModified() const;

    QString imageFileName() const;
    QString imageName() const;

    qint32 trackCount() const;
    qint32 imageSize() const;

    bool insertImage(const QString& fileName);
    bool ejectImage();
    bool saveImage();

    bool readSector(quint8 track, quint8 sector, QByteArray& data) const;
    bool writeSector(quint8 track, quint8 sector, const QByteArray& data);

    static quint8 sectorsOnTrack(quint8 track);
    static qint32 sectorOffset(quint8 track, quint8 sector);

private:
    bool setError(Error error) const;
    void clearError() const;
    void resetImage();

    State m_state = State::Empty;
    mutable Error m_error = Error::None;

    QString m_imageFileName;
    QString m_imageName;

    QByteArray m_imageData;

    qint32 m_trackCount = 0;
    bool m_hasErrorBytes = false;
};
