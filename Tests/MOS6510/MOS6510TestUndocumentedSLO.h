#pragma once

#include "MOS6510TestBase.h"

#include <QObject>


class MOS6510TestUndocumentedSLO : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedSLO();
    virtual ~MOS6510TestUndocumentedSLO();

private slots:
    void testZeroPage_data();
    void testZeroPage();

    void testZeroPageX();
    void testZeroPageXWrap();

    void testAbsolute();

    void testAbsoluteX();
    void testAbsoluteXPageCross();

    void testAbsoluteY();
    void testAbsoluteYPageCross();

    void testIndexedIndirect();
    void testIndexedIndirectPointerWrap();

    void testIndirectIndexed();
    void testIndirectIndexedPageCross();
    void testIndirectIndexedPointerWrap();

    void testExhaustive();
};
