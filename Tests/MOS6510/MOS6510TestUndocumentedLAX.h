#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedLAX : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct LAXResult
    {
        quint8 accumulator;
        quint8 x;
        bool zero;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedLAX();
    virtual ~MOS6510TestUndocumentedLAX();

private:
    static LAXResult referenceLAX(
        quint8 operand);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const LAXResult& result);

private slots:
    void testZeroPage_data();
    void testZeroPage();

    void testZeroPageY();
    void testZeroPageYWrap();

    void testAbsolute();

    void testAbsoluteY();
    void testAbsoluteYPageCross();

    void testIndirectX();
    void testIndirectXZeroPageWrap();

    void testIndirectY();
    void testIndirectYPageCross();
    void testIndirectYZeroPageWrap();

    void testExhaustive();
};
