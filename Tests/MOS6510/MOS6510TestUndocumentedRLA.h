#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedRLA : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct RLAResult
    {
        quint8 accumulator;
        quint8 memory;
        bool carry;
        bool zero;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedRLA();
    virtual ~MOS6510TestUndocumentedRLA();

private:
    static RLAResult referenceRLA(
        quint8 accumulator,
        quint8 memory,
        bool carry);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const RLAResult& result);

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

    void testExhaustive();
};
