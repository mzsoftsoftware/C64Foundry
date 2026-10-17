#pragma once

#include <QObject>


class C64MachineIECTest : public QObject
{
    Q_OBJECT

public:
    explicit C64MachineIECTest();
    virtual ~C64MachineIECTest();

private slots:
    void testInitialIECLineStates();
    void testCIA2IECOutputs();
    void testCIA2DataDirection();
    void testExternalIECInputs();
    void testVICBankBitsUnaffected();
    void testResetAndPowerOn();
};
