#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedSHY
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedSHY();
    virtual ~MOS6510TestUndocumentedSHY();

private slots:
    void testSHYAbsoluteX_data();
    void testSHYAbsoluteX();

    void testSHYAbsoluteXFlags();

    void testSHYAbsoluteXCycles();
    void testSHYAbsoluteXCyclesPageCrossing();
};
