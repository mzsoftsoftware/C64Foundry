#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedXAA
    : public QObject
    , public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedXAA();
    virtual ~MOS6510TestUndocumentedXAA();

private slots:
    void testXAAImmediate_data();
    void testXAAImmediate();

    void testXAAFlags();

    void testXAAImmediateCycles();
};
