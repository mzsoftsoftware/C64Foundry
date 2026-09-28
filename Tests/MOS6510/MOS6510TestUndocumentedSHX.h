#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedSHX
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedSHX();
    virtual ~MOS6510TestUndocumentedSHX();

private slots:
    void testSHXAbsoluteY_data();
    void testSHXAbsoluteY();

    void testSHXAbsoluteYFlags();

    void testSHXAbsoluteYCycles();
    void testSHXAbsoluteYCyclesPageCrossing();
};
