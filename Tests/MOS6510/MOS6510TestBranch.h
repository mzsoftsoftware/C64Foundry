#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestBranch : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestBranch();
    virtual ~MOS6510TestBranch();

private slots:
    void testBranchNotTaken_data();
    void testBranchNotTaken();

    void testBranchTakenForward_data();
    void testBranchTakenForward();

    void testBranchTakenBackward_data();
    void testBranchTakenBackward();

    void testBranchTakenForwardPageCrossing_data();
    void testBranchTakenForwardPageCrossing();

    void testBranchTakenBackwardPageCrossing_data();
    void testBranchTakenBackwardPageCrossing();
};