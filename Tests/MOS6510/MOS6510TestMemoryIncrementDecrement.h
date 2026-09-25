#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestMemoryIncrementDecrement : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestMemoryIncrementDecrement();
    virtual ~MOS6510TestMemoryIncrementDecrement();

private slots:
    void init();

    // INC
    void testIncrementZeroPage_data();
    void testIncrementZeroPage();

    void testIncrementZeroPageX_data();
    void testIncrementZeroPageX();

    void testIncrementZeroPageXWrapAround();

    void testIncrementAbsolute_data();
    void testIncrementAbsolute();

    void testIncrementAbsoluteX_data();
    void testIncrementAbsoluteX();

    void testIncrementAbsoluteXWithPageCrossing();

    void testDecrementZeroPage_data();
    void testDecrementZeroPage();

    void testDecrementZeroPageX_data();
    void testDecrementZeroPageX();
    void testDecrementZeroPageXWrapAround();

    void testDecrementAbsolute_data();
    void testDecrementAbsolute();

    void testDecrementAbsoluteX_data();
    void testDecrementAbsoluteX();
    void testDecrementAbsoluteXWithPageCrossing();
};
