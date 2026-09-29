#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedAHX
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedAHX();
    virtual ~MOS6510TestUndocumentedAHX();

private slots:
    void testAHXAbsoluteY_data();
    void testAHXAbsoluteY();

    void testAHXIndirectY_data();
    void testAHXIndirectY();

    void testAHXRegistersAndFlags();

    void testAHXAbsoluteYCycles();
    void testAHXAbsoluteYCyclesPageCrossing();

    void testAHXIndirectYCycles();
    void testAHXIndirectYCyclesPageCrossing();
    void testAHXIndirectYZeroPageWrap();
};
