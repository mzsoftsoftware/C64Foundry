#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedASR : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct ASRResult
    {
        quint8 accumulator;
        bool carry;
        bool zero;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedASR();
    virtual ~MOS6510TestUndocumentedASR();

private:
    static ASRResult referenceASR(
        quint8 accumulator,
        quint8 operand);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const ASRResult& result);

private slots:
    void testImmediate_data();
    void testImmediate();

    void testExhaustive();
};
