#include "IECBusTest.h"

#include <QTest>

#include "C64/IEC/IECBus.h"


IECBusTest::IECBusTest()
{
}

IECBusTest::~IECBusTest()
{
}

void IECBusTest::testInitialLineStates()
{
    IECBus bus;

    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

void IECBusTest::testSingleParticipant()
{
    IECBus bus;

    const IECBus::ParticipantId participant = bus.addParticipant();

    QVERIFY(bus.setLineLow(participant, IECBus::Line::DATA, true));

    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));
    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
}

void IECBusTest::testMultipleParticipants()
{
    IECBus bus;

    const IECBus::ParticipantId participant1 = bus.addParticipant();
    const IECBus::ParticipantId participant2 = bus.addParticipant();

    QVERIFY(bus.setLineLow(participant1, IECBus::Line::DATA, true));
    QVERIFY(bus.setLineLow(participant2, IECBus::Line::DATA, true));

    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));

    QVERIFY(bus.setLineLow(participant1, IECBus::Line::DATA, false));
    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));

    QVERIFY(bus.setLineLow(participant2, IECBus::Line::DATA, false));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

void IECBusTest::testIndependentLines()
{
    IECBus bus;

    const IECBus::ParticipantId participant1 = bus.addParticipant();
    const IECBus::ParticipantId participant2 = bus.addParticipant();

    QVERIFY(bus.setLineLow(participant1, IECBus::Line::ATN, true));
    QVERIFY(bus.setLineLow(participant2, IECBus::Line::CLK, true));

    QVERIFY(!bus.lineHigh(IECBus::Line::ATN));
    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));

    QVERIFY(bus.setLineLow(participant1, IECBus::Line::ATN, false));

    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));
}

void IECBusTest::testReleaseLine()
{
    IECBus bus;

    const IECBus::ParticipantId participant = bus.addParticipant();

    QVERIFY(bus.setLineLow(participant, IECBus::Line::DATA, true));
    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));

    QVERIFY(bus.setLineLow(participant, IECBus::Line::DATA, false));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));

    // Releasing an already released line is valid.
    QVERIFY(bus.setLineLow(participant, IECBus::Line::DATA, false));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

void IECBusTest::testRemoveParticipant()
{
    IECBus bus;

    const IECBus::ParticipantId participant1 = bus.addParticipant();
    const IECBus::ParticipantId participant2 = bus.addParticipant();

    QVERIFY(bus.setLineLow(participant1, IECBus::Line::ATN, true));
    QVERIFY(bus.setLineLow(participant1, IECBus::Line::DATA, true));
    QVERIFY(bus.setLineLow(participant2, IECBus::Line::DATA, true));

    QVERIFY(bus.removeParticipant(participant1));

    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));

    QVERIFY(bus.removeParticipant(participant2));

    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

void IECBusTest::testInvalidParticipant()
{
    IECBus bus;

    const IECBus::ParticipantId participant = bus.addParticipant();

    QVERIFY(bus.removeParticipant(participant));

    QVERIFY(!bus.setLineLow(participant, IECBus::Line::DATA, true));
    QVERIFY(!bus.removeParticipant(participant));

    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

