#pragma once

#include <QThread>

class EmulatorWorker;


class EmulatorThread : public QThread
{
    Q_OBJECT
public:
    explicit EmulatorThread(EmulatorWorker* ptrWorker, QObject *parent = nullptr);
    virtual ~EmulatorThread();

protected:
    void run() override;

private:
    EmulatorWorker* m_ptrWorker = nullptr;
};
