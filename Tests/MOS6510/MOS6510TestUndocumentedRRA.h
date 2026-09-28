#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedRRA : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct RRAResult
    {
        quint8 accumulator;
        quint8 memory;
        bool carry;
        bool zero;
        bool overflow;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedRRA();
    virtual ~MOS6510TestUndocumentedRRA();

private:
    static RRAResult referenceRRA(
        quint8 accumulator,
        quint8 memory,
        bool carry,
        bool decimal);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const RRAResult& result);

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

    void testIndirectX();
    void testIndirectXZeroPageWrap();

    void testIndirectY();
    void testIndirectYPageCross();
    void testIndirectYZeroPageWrap();

    void testExhaustiveBinary();
    void testExhaustiveDecimal();
};
