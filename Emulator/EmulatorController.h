#pragma once

#include <QObject>

#include "EmulatorSpeed.h"

#include "C64/C64ROMSet.h"
#include "Input/InputEvent.h"

class EmulatorThread;
class EmulatorWorker;
class C64Machine;


class EmulatorController : public QObject
{
    Q_OBJECT
public:
    explicit EmulatorController(QObject *parent);
    virtual ~EmulatorController();

    // Getter
    C64Machine* machine() const;

public slots:
    void loadROMSet(const C64ROMSet& romSet);
    void input(const InputEvent& event);

    void start();
    void stop();
    void reset();
    void setSpeed(Emulator::Speed speed);

signals:
    void romSetLoaded(bool bLoaded);

    void runningChanged(bool bRunning);
    void speedChanged(Emulator::Speed speed);
    void statisticsChanged(quint64 cycles, quint64 currentCyclesPerSecond, quint64 averageCyclesPerSecond);

private:
    EmulatorThread* m_ptrThread = nullptr;
    EmulatorWorker* m_ptrWorker = nullptr;
};
