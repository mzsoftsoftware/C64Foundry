#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedNOP : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedNOP();
    virtual ~MOS6510TestUndocumentedNOP();

private slots:
    void testImplied_data();
    void testImplied();

    void testImmediate_data();
    void testImmediate();

    void testZeroPage_data();
    void testZeroPage();

    void testZeroPageX_data();
    void testZeroPageX();

    void testAbsolute_data();
    void testAbsolute();

    void testAbsoluteX_data();
    void testAbsoluteX();

    void testAbsoluteXPageCross_data();
    void testAbsoluteXPageCross();

    void testZeroPageXWrapAround_data();
    void testZeroPageXWrapAround();
};