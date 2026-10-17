#pragma once

#include <QObject>


class IECBusTest : public QObject
{
    Q_OBJECT

public:
    explicit IECBusTest();
    virtual ~IECBusTest();

private slots:
    void testInitialLineStates();
    void testSingleParticipant();
    void testMultipleParticipants();
    void testIndependentLines();
    void testReleaseLine();
    void testRemoveParticipant();
    void testInvalidParticipant();

    void testSRQLine();
    void testRESETLine();
    void testNewLineIndependence();
    void testNewLinesMultipleParticipants();
    void testNewLinesRemoveParticipant();
};

