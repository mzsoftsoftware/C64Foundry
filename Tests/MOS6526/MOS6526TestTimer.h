#pragma once

#include <QObject>


class MOS6526TestTimer : public QObject
{
    Q_OBJECT

public:
    explicit MOS6526TestTimer();
    virtual ~MOS6526TestTimer();

private slots:
    void testTimerALatchLoadWhenStopped();
    void testTimerAForceLoad();
    void testTimerAStopped();
    void testTimerACount();
    void testTimerAUnderflowReload();
    void testTimerAInterruptMask();
    void testInterruptControlRegisterReadClearsStatus();
    void testTimerAIrq();
    void testTimerAInterruptWithoutMask();
    void testInterruptMaskClear();
    void testTimerALatchWriteWhileRunning();
    void testTimerAOneShot();
};
