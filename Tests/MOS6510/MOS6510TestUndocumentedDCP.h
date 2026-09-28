#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedDCP : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct DCPResult
    {
        quint8 memory;
        bool carry;
        bool zero;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedDCP();
    virtual ~MOS6510TestUndocumentedDCP();

private:
    static DCPResult referenceDCP(
        quint8 accumulator,
        quint8 memory);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const DCPResult& result);

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

    void testExhaustive();
};
