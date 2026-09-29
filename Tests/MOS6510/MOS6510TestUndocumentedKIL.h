#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedKIL
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedKIL();
    virtual ~MOS6510TestUndocumentedKIL();

private slots:
    void testKIL_data();
    void testKIL();

    void testKILRemainsStopped();
};
