#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestStack : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestStack();
    virtual ~MOS6510TestStack();

private:
    void initializeRegisters();

private slots:
    void testPush_data();
    void testPush();

    void testPushWrap_data();
    void testPushWrap();

    void testPullAccumulator_data();
    void testPullAccumulator();

    void testPullAccumulatorWrap();

    void testPullStatus_data();
    void testPullStatus();

    void testPullStatusWrap();
};