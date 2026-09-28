#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedISB : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct ISBResult
    {
        quint8 memory;
        quint8 accumulator;
        bool carry;
        bool zero;
        bool overflow;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedISB();
    virtual ~MOS6510TestUndocumentedISB();

private:
    static ISBResult referenceISB(
        quint8 accumulator,
        quint8 memory,
        bool carry,
        bool decimal);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const ISBResult& result);

private slots:
    void testZeroPage_data();
    void testZeroPage();

    void testZeroPageX();

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
