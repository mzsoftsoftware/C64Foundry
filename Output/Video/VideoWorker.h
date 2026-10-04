#pragma once

#include <QObject>
#include <QMutex>
#include <QImage>

class C64Machine;


class VideoWorker : public QObject
{
    Q_OBJECT
public:
    explicit VideoWorker(C64Machine* ptrMachine, QObject* parent = nullptr);
    virtual ~VideoWorker();

    // Operations
    void run();

    void requestShutdown();

public slots:
    void frameTaken();

signals:
    void frameReady(const QImage& image);

private:
    QImage convertFrame(const quint8* ptrFrame) const;

private:
    C64Machine* m_ptrMachine = nullptr;

    quint16 m_frameWidth = 0;
    quint16 m_frameHeight = 0;

    QMutex m_mutex;
    bool m_bShutdownRequested = false;
    bool m_bFramePending = false;
};
