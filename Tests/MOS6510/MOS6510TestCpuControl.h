#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestCpuControl : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestCpuControl();
    virtual ~MOS6510TestCpuControl();

private slots:
    void testReset();
    void testResetVector();
    void testResetStackPointer();
    void testResetStatus();
    void testResetCycles();

    void testIrq();
    void testIrqMasked();
    void testIrqStack();
    void testIrqStatus();
    void testIrqCycles();
    void testIrqAfterCli();
    void testIrqAfterSei();
    void testIrqAfterPlp();
    void testIrqAfterRti();
    void testIrqReleased();
    void testIrqBranchNotTaken();
    void testIrqBranchTaken();
    void testIrqBranchTakenPageCrossing();

    void testNmi();
    void testNmiIgnoredInterruptDisable();
    void testNmiStack();
    void testNmiStatus();
    void testNmiCycles();
    void testNmiEdgeTriggered();
    void testNmiSecondEdge();
    void testNmiDuringInstruction();
    void testNmiDuringBranchPageCrossing();

    void testNmiPriorityOverIrq();
    void testNmiDuringIrqBeforeVector();
    void testNmiDuringIrqTooLateForVector();

    void testNmiBeforeBrk();
    void testNmiDuringBrkBeforeVector();
    void testNmiDuringBrkTooLateForVector();

    void testNmiLateDuringIrq();
    void testNmiLateDuringBrk();
    void testNmiLostDuringIrqVectorFetch();

    void testNmiPriorityOverIrqHandlerStartsNormally();
    void testNmiTooLateDuringTwoCycleInstruction();
    void testNmiTooLateDuringMultiCycleInstruction();

    void testNmiBranchNotTaken();
    void testNmiBranchTaken();
};