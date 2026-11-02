#pragma once

#include <QObject>


class MOS6526TestRegister : public QObject
{
    Q_OBJECT

public:
    explicit MOS6526TestRegister();
    virtual ~MOS6526TestRegister();

private slots:
    void testRegisterMirroring();
    void testControlRegisterAWriteRead();
    void testControlRegisterBWriteRead();
    void testTimerAForceLoadBitIsNotStored();
    void testTimerBForceLoadBitIsNotStored();
    void testInterruptMaskSet();
    void testInterruptMaskClear();
    void testMultipleInterruptSources();
    void testResetClearsRegistersAndInterrupts();
    void testInitializeResetsCIA();

    void testSerialDataRegisterWriteRead();
    void testSerialDataRegisterMirroring();
    void testSerialDataRegisterReset();
    void testSerialPortOutputMode();
    void testSerialPortInputMode();
    void testSerialDataRegisterAccessibleInBothModes();
    void testSerialPortInputLevel();
    void testSerialPortOutputResetState();
};
