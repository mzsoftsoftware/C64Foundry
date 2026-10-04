#pragma once

#include <QObject>
#include <QMutex>

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

private:
    C64Machine* m_ptrMachine = nullptr;

    QMutex m_mutex;
    bool m_bShutdownRequested = false;
};
