#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedAXS
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedAXS();
    virtual ~MOS6510TestUndocumentedAXS();

private slots:
    void testAXSImmediate_data();
    void testAXSImmediate();

    void testAXSImmediateFlags();

    void testAXSImmediateCycles();
};
