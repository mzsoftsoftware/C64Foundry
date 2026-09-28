#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedARR : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedARR();
    virtual ~MOS6510TestUndocumentedARR();

private:
    struct ARRResult
    {
        quint8 accumulator;
        bool carry;
        bool zero;
        bool overflow;
        bool negative;
    };

    static ARRResult referenceARR(
        quint8 accumulator,
        quint8 operand,
        bool carryIn,
        bool decimal);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const ARRResult& result);

private slots:
    void testImmediate_data();
    void testImmediate();

    void testImmediateDecimal_data();
    void testImmediateDecimal();

    void testExhaustive();
};
