#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestLogical : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestLogical();
    virtual ~MOS6510TestLogical();

private:
    void initializeRegisters();
    quint8 expectedLogicalStatus(quint8 status, quint8 value) const;

private slots:
    void testLogicalImmediate_data();
    void testLogicalImmediate();

    void testLogicalZeroPage_data();
    void testLogicalZeroPage();

    void testLogicalZeroPageIndexed_data();
    void testLogicalZeroPageIndexed();

    void testLogicalAbsolute_data();
    void testLogicalAbsolute();

    void testLogicalAbsoluteIndexed_data();
    void testLogicalAbsoluteIndexed();

    void testLogicalAbsoluteIndexedPageCrossing_data();
    void testLogicalAbsoluteIndexedPageCrossing();
};
