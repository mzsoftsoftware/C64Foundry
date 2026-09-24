#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestIncrementDecrement : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestIncrementDecrement();
    virtual ~MOS6510TestIncrementDecrement();

private:
    void initializeRegisters();

private slots:
    void testInx_data();
    void testInx();

    void testIny_data();
    void testIny();

    void testDex_data();
    void testDex();

    void testDey_data();
    void testDey();
};
