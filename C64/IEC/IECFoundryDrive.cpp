#include "IECFoundryDrive.h"

#include <QFile>
#include <QFileInfo>


IECFoundryDrive::IECFoundryDrive()
{
}

IECFoundryDrive::~IECFoundryDrive()
{
}

IECFoundryDrive::State IECFoundryDrive::state() const
{
    return m_state;
}

IECFoundryDrive::Error IECFoundryDrive::error() const
{
    return m_error;
}

bool IECFoundryDrive::hasImage() const
{
    return !m_imageData.isEmpty();
}

bool IECFoundryDrive::isModified() const
{
    return m_state == State::Modified;
}

QString IECFoundryDrive::imageFileName() const
{
    return m_imageFileName;
}

QString IECFoundryDrive::imageName() const
{
    return m_imageName;
}

qint32 IECFoundryDrive::trackCount() const
{
    return m_trackCount;
}

qint32 IECFoundryDrive::imageSize() const
{
    return m_imageData.size();
}

quint8 IECFoundryDrive::sectorsOnTrack(const quint8 track)
{
    if (track < 1 || track > 40)
        return 0;

    if (track <= 17)
        return 21;

    if (track <= 24)
        return 19;

    if (track <= 30)
        return 18;

    return 17;
}

qint32 IECFoundryDrive::sectorOffset(const quint8 track, const quint8 sector)
{
    if (track < 1 || track > 40 || sector >= sectorsOnTrack(track))
        return -1;

    qint32 offset = 0;

    for (quint8 currentTrack = 1; currentTrack < track; ++currentTrack)
        offset += sectorsOnTrack(currentTrack) * 256;

    return offset + static_cast<qint32>(sector) * 256;
}

bool IECFoundryDrive::insertImage(const QString& fileName)
{
    clearError();

    QFile file(fileName);

    if (!file.exists())
        return setError(Error::FileNotFound);

    if (!file.open(QIODevice::ReadOnly))
        return setError(Error::FileOpenFailed);

    const QByteArray data = file.readAll();

    if (file.error() != QFileDevice::NoError)
        return setError(Error::FileOpenFailed);

    qint32 tracks = 0;
    bool hasErrorBytes = false;

    switch (data.size())
    {
    case 174848:
        tracks = 35;
        break;

    case 175531:
        tracks = 35;
        hasErrorBytes = true;
        break;

    case 196608:
        tracks = 40;
        break;

    case 197376:
        tracks = 40;
        hasErrorBytes = true;
        break;

    default:
        return setError(Error::UnsupportedImage);
    }

    resetImage();

    m_imageData = data;
    m_imageFileName = QFileInfo(fileName).absoluteFilePath();
    m_imageName = QFileInfo(fileName).fileName();
    m_trackCount = tracks;
    m_hasErrorBytes = hasErrorBytes;
    m_state = State::Ready;

    return true;
}

bool IECFoundryDrive::ejectImage()
{
    clearError();

    resetImage();

    return true;
}

bool IECFoundryDrive::saveImage()
{
    clearError();

    if (!hasImage())
        return setError(Error::InvalidImage);

    if (m_imageFileName.isEmpty())
        return setError(Error::FileNotFound);

    QFile file(m_imageFileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return setError(Error::WriteFailed);

    if (file.write(m_imageData) != m_imageData.size())
        return setError(Error::WriteFailed);

    if (!file.flush())
        return setError(Error::WriteFailed);

    m_state = State::Ready;

    return true;
}

bool IECFoundryDrive::readSector(const quint8 track, const quint8 sector, QByteArray& data) const
{
    clearError();

    if (!hasImage())
        return setError(Error::InvalidImage);

    if (track > m_trackCount)
        return setError(Error::InvalidTrack);

    const qint32 offset = sectorOffset(track, sector);

    if (offset < 0)
        return setError(Error::InvalidSector);

    if (offset + 256 > m_imageData.size())
        return setError(Error::InvalidSector);

    data = m_imageData.mid(offset, 256);

    return true;
}

bool IECFoundryDrive::writeSector(const quint8 track, const quint8 sector, const QByteArray& data)
{
    clearError();

    if (!hasImage())
        return setError(Error::InvalidImage);

    if (track > m_trackCount)
        return setError(Error::InvalidTrack);

    if (data.size() != 256)
        return setError(Error::InvalidSector);

    const qint32 offset = sectorOffset(track, sector);

    if (offset < 0)
        return setError(Error::InvalidSector);

    if (offset + data.size() > m_imageData.size())
        return setError(Error::InvalidSector);

    m_imageData.replace(offset, data.size(), data);

    m_state = State::Modified;

    return true;
}

bool IECFoundryDrive::setError(const Error error) const
{
    m_error = error;
    return false;
}

void IECFoundryDrive::clearError() const
{
    m_error = Error::None;
}

void IECFoundryDrive::resetImage()
{
    m_imageData.clear();
    m_imageFileName.clear();
    m_imageName.clear();

    m_trackCount = 0;
    m_hasErrorBytes = false;
    m_state = State::Empty;
    m_error = Error::None;
}
