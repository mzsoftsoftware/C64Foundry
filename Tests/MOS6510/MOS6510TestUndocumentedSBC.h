#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedSBC : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedSBC();
    virtual ~MOS6510TestUndocumentedSBC();

private slots:
    void testImmediate_data();
    void testImmediate();

    void testImmediateDecimal_data();
    void testImmediateDecimal();
};