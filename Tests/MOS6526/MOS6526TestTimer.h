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
    void testTimerACntClock();
    void testTimerBCounterLoadWhenStopped();
    void testTimerBForceLoad();
    void testTimerBStopped();
    void testTimerBCount();
    void testTimerBUnderflowReload();
    void testTimerBOneShot();
    void testTimerBInterrupt();
    void testTimerBCntClock();
    void testTimerBTimerAUnderflowClock();
    void testTimerBTimerAUnderflowClockWhileCntHigh();

    void testTimerALatchReloadAfterLatchChange();
    void testTimerACntClockDoesNotCountPhi2();
    void testTimerBReloadAfterLatchChange();
    void testTimerBTimerAUnderflowDoesNotCountPhi2();
    void testTimerBTimerAUnderflowWhileCntLow();

    void testTimerAOutputPulse();
    void testTimerAOutputToggle();
    void testTimerAOutputOverridesDDRB();
    void testTimerAOutputToggleStartState();
    void testTimerAOutputReset();

    void testTimerBOutputPulse();
    void testTimerBOutputToggle();
    void testTimerBOutputOverridesDDRB();
    void testTimerBOutputTimerAUnderflow();
    void testTimerBOutputReset();
};
