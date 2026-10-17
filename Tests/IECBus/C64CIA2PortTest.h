#pragma once

#include <QObject>


class C64CIA2PortTest : public QObject
{
    Q_OBJECT

public:
    explicit C64CIA2PortTest();
    virtual ~C64CIA2PortTest();

private slots:
    void testInitialIECLineStates();

    void testClockOutput();
    void testDataOutput();
    void testAttentionOutput();

    void testClockInput();
    void testDataInput();

    void testDataDirectionRegister();
    void testIndependentIECSignals();

    void testExternalParticipantPullsClockLow();
    void testExternalParticipantPullsDataLow();

    void testVICBankBitsUnaffected();
};
