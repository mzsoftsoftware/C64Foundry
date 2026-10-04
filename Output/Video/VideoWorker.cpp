#include "VideoWorker.h"

#include <QThread>

#include "C64/C64Machine.h"


namespace
{
constexpr QRgb C64Palette[16] =
    {
        0xFF000000, // Black
        0xFFFFFFFF, // White
        0xFF813338, // Red
        0xFF75CEC8, // Cyan
        0xFF8E3C97, // Purple
        0xFF56AC4D, // Green
        0xFF2E2C9B, // Blue
        0xFFEDF171, // Yellow
        0xFF8E5029, // Orange
        0xFF553800, // Brown
        0xFFC46C71, // Light red
        0xFF4A4A4A, // Dark gray
        0xFF7B7B7B, // Gray
        0xFFA9FF9F, // Light green
        0xFF706DEB, // Light blue
        0xFFB2B2B2  // Light gray
};
}


VideoWorker::VideoWorker(C64Machine* ptrMachine, QObject* parent)
    : QObject{parent}
    , m_ptrMachine{ptrMachine}
{
}
VideoWorker::~VideoWorker()
{
}

void VideoWorker::run()
{
    //
    // The video format remains constant for the
    // lifetime of the emulated machine.
    //
    m_timing = m_ptrMachine->timing();
    m_frameWidth =
        static_cast<quint16>(
            m_timing.cyclesPerLine * 8);

    m_frameHeight =
        static_cast<quint16>(
            m_timing.linesPerFrame);

    m_firstPartWidth =
        qMin(
            m_timing.visibleWidth,
            static_cast<quint16>(
                m_frameWidth -
                m_timing.visibleFirstPixel));

    m_secondPartWidth =
        m_timing.visibleWidth -
        m_firstPartWidth;

    while (true)
    {
        bool bShutdownRequested = false;
        bool bFramePending = false;

        {
            QMutexLocker locker(&m_mutex);
            bShutdownRequested = m_bShutdownRequested;
            bFramePending = m_bFramePending;
        }

        if (bShutdownRequested)
            break;

        //
        // Do not acquire another frame while the
        // previous image is still owned by the GUI.
        //
        if (bFramePending)
        {
            QThread::msleep(1);
            continue;
        }

        const quint8* ptrFrame = m_ptrMachine->acquireVideoFrame();
        if (ptrFrame == nullptr)
        {
            QThread::msleep(1);
            continue;
        }

        //
        // Convert the VIC-II color indices into
        // a displayable RGB image.
        //
        const QImage image = convertFrame(ptrFrame);
        {
            QMutexLocker locker(&m_mutex);
            m_bFramePending = true;
        }
        emit frameReady(image);
    }
}

void VideoWorker::requestShutdown()
{
    QMutexLocker locker(&m_mutex);
    m_bShutdownRequested = true;
}

QImage VideoWorker::convertFrame(const quint8* ptrFrame) const
{
    QImage image(
        m_timing.visibleWidth,
        m_timing.visibleHeight,
        QImage::Format_RGB32);

    for (quint16 y = 0;
         y < m_timing.visibleHeight;
         ++y)
    {
        quint16 sourceY =
            m_timing.visibleFirstLine + y;

        //
        // The visible area may wrap around to the
        // beginning of the raster frame.
        //
        if (sourceY >= m_frameHeight)
            sourceY -= m_frameHeight;

        const quint8* ptrSource =
            ptrFrame +
            static_cast<quint32>(sourceY) *
                m_frameWidth;

        QRgb* ptrTarget =
            reinterpret_cast<QRgb*>(
                image.scanLine(y));

        //
        // First part of the visible raster line.
        //
        const quint8* ptrSourceFirst =
            ptrSource +
            m_timing.visibleFirstPixel;

        for (quint16 x = 0;
             x < m_firstPartWidth;
             ++x)
        {
            ptrTarget[x] =
                C64Palette[
                    ptrSourceFirst[x] & 0x0F
            ];
        }

        //
        // The visible area may wrap around to the
        // beginning of the raster line.
        //
        for (quint16 x = 0;
             x < m_secondPartWidth;
             ++x)
        {
            ptrTarget[m_firstPartWidth + x] =
                C64Palette[
                    ptrSource[x] & 0x0F
            ];
        }
    }

    return image;
}

void VideoWorker::frameTaken()
{
    QMutexLocker locker(&m_mutex);
    m_bFramePending = false;
}
