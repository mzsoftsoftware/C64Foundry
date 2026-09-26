#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestDormann : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestDormann();
    virtual ~MOS6510TestDormann();

private slots:
    void testLoadDormannTest();
    void testStartDormannTest();
    void testDormannTest();
    void testDormannTestPerformance();
};