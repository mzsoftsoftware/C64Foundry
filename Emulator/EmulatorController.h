#pragma once

#include <QObject>

#include "EmulatorSpeed.h"

class EmulatorThread;
class EmulatorWorker;


class EmulatorController : public QObject
{
    Q_OBJECT
public:
    explicit EmulatorController(QObject *parent);
    virtual ~EmulatorController();

public slots:
    void start();
    void stop();
    void reset();
    void setSpeed(Emulator::Speed speed);

signals:
    void runningChanged(bool bRunning);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 cyclesPerSecond);

private:
    EmulatorThread* m_ptrThread = nullptr;
    EmulatorWorker* m_ptrWorker = nullptr;
};
