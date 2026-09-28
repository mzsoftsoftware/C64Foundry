#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedLaxImmediate
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedLaxImmediate();
    virtual ~MOS6510TestUndocumentedLaxImmediate();

private slots:
    void testLaxImmediate_data();
    void testLaxImmediate();

    void testLaxImmediateFlags();

    void testLaxImmediateCycles();
};
