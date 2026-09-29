#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedTAS
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedTAS();
    virtual ~MOS6510TestUndocumentedTAS();

private slots:
    void testTASAbsoluteY_data();
    void testTASAbsoluteY();

    void testTASRegistersAndFlags();

    void testTASAbsoluteYCycles();
    void testTASAbsoluteYCyclesPageCrossing();
};