#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestPort : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestPort();
    virtual ~MOS6510TestPort();

private slots:
    void testDataDirectionRegisterWrite();
    void testDataDirectionRegisterRead();

    void testDataRegisterWrite();
    void testDataRegisterReadOutputs();

    void testDataDirectionRegisterWriteDataBusNotDriven();
    void testDataRegisterWriteDataBusNotDriven();

    void testDataDirectionRegisterWriteBusCycle();
    void testDataRegisterWriteBusCycle();

    void testDataDirectionRegisterWritePreservesDataBusValue();
    void testDataRegisterWritePreservesDataBusValue();

    void testDataDirectionRegisterWriteWritesDataBusValueToRAM();
    void testDataRegisterWriteWritesDataBusValueToRAM();

    void testImmediateLoadPcWrap();
    void testIndexedIndirectPointerWrap();
    void testIndirectIndexedPointerWrap();

    void testStoreAccumulatorToDataDirectionRegister();
    void testStoreXToDataDirectionRegister();
    void testStoreYToDataDirectionRegister();

    void testIndexedIndirectStorePointerWrap();
    void testIndirectIndexedStorePointerWrap();

    void testDataDirectionRegisterReadInternalValueExternalRamValue();
    void testDataRegisterReadInternalValueExternalRamValue();

    void testDataRegisterBit6FalloffAfterOutputToInput();
    void testDataRegisterBit7FalloffAfterOutputToInput();
    void testDataRegisterBits67LowAfterOutputToInput();

};
