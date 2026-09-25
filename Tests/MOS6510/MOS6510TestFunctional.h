#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestFunctional : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestFunctional();
    virtual ~MOS6510TestFunctional();

private slots:
    void testLoadFunctionalTest();
    void testStartFunctionalTest();
    void testFunctionalTest();
    void testFunctionalTestPerformance();
};