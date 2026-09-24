#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestDecimal : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestDecimal();
    virtual ~MOS6510TestDecimal();

private:
    void initializeRegisters();

private slots:
    void testSed_data();
    void testSed();

    void testCld_data();
    void testCld();

    void testDecimalModeSwitch();


    void testAdcImmediate_data();
    void testAdcImmediate();

    void testSbcImmediate_data();
    void testSbcImmediate();


    void testAdcDecimalExhaustive();
    void testSbcDecimalExhaustive();
};
