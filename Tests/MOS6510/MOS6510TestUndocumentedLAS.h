#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestUndocumentedLAS : public QObject, public MOS6510TestBase
{
    Q_OBJECT

private:
    struct LASResult
    {
        quint8 value;
        bool zero;
        bool negative;
    };

public:
    explicit MOS6510TestUndocumentedLAS();
    virtual ~MOS6510TestUndocumentedLAS();

private:
    static LASResult referenceLAS(
        quint8 memory,
        quint8 stackPointer);

    static quint8 expectedStatus(
        quint8 initialStatus,
        const LASResult& result);

private slots:
    void testAbsoluteY_data();
    void testAbsoluteY();

    void testAbsoluteYPageCross();

    void testExhaustive();
};
