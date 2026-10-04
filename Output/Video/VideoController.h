#pragma once

#include <QObject>

class VideoThread;
class VideoWorker;
class C64Machine;


class VideoController : public QObject
{
    Q_OBJECT
public:
    explicit VideoController(C64Machine* ptrMachine, QObject* parent);
    virtual ~VideoController();

private:
    VideoWorker* m_ptrWorker = nullptr;
    VideoThread* m_ptrThread = nullptr;
};
