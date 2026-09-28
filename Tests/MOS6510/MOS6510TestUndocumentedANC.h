#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedANC : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct ANCResult
    {
        quint8 accumulator;
        bool carry;
        bool zero;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedANC();
    virtual ~MOS6510TestUndocumentedANC();

private:
    static ANCResult referenceANC(
        quint8 accumulator,
        quint8 operand);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const ANCResult& result);

private slots:
    void testImmediate_data();
    void testImmediate();

    void testExhaustive();
};
