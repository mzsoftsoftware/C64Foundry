#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestShiftRotate : public QObject,
                               public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestShiftRotate();
    virtual ~MOS6510TestShiftRotate();

private slots:
    void init();

    // ASL
    void testAslAccumulator_data();
    void testAslAccumulator();

    void testAslZeroPage_data();
    void testAslZeroPage();

    void testAslZeroPageX_data();
    void testAslZeroPageX();
    void testAslZeroPageXWrapAround();

    void testAslAbsolute_data();
    void testAslAbsolute();

    void testAslAbsoluteX_data();
    void testAslAbsoluteX();
    void testAslAbsoluteXWithPageCrossing();

    // LSR
    void testLsrAccumulator_data();
    void testLsrAccumulator();

    void testLsrZeroPage_data();
    void testLsrZeroPage();

    void testLsrZeroPageX_data();
    void testLsrZeroPageX();
    void testLsrZeroPageXWrapAround();

    void testLsrAbsolute_data();
    void testLsrAbsolute();

    void testLsrAbsoluteX_data();
    void testLsrAbsoluteX();
    void testLsrAbsoluteXWithPageCrossing();

    // ROL
    void testRolAccumulator_data();
    void testRolAccumulator();

    void testRolZeroPage_data();
    void testRolZeroPage();

    void testRolZeroPageX_data();
    void testRolZeroPageX();
    void testRolZeroPageXWrapAround();

    void testRolAbsolute_data();
    void testRolAbsolute();

    void testRolAbsoluteX_data();
    void testRolAbsoluteX();
    void testRolAbsoluteXWithPageCrossing();

    // ROR
    void testRorAccumulator_data();
    void testRorAccumulator();

    void testRorZeroPage_data();
    void testRorZeroPage();

    void testRorZeroPageX_data();
    void testRorZeroPageX();
    void testRorZeroPageXWrapAround();

    void testRorAbsolute_data();
    void testRorAbsolute();

    void testRorAbsoluteX_data();
    void testRorAbsoluteX();
    void testRorAbsoluteXWithPageCrossing();
};