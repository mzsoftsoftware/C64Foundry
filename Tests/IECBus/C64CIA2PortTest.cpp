#include "C64CIA2PortTest.h"

#include <QTest>

#include "C64/CIA/MOS6526.h"
#include "C64/IEC/IECBus.h"
#include "C64/CIA/C64CIA2Port.h"


C64CIA2PortTest::C64CIA2PortTest()
{
}

C64CIA2PortTest::~C64CIA2PortTest()
{
}


void C64CIA2PortTest::testInitialIECLineStates()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

void C64CIA2PortTest::testClockOutput()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    //
    // PA4 = CLK OUT.
    // CIA2 output high pulls IEC CLK low.
    //
    cia.writeRegister(0x00, 0x00);
    cia.writeRegister(0x02, 0x10);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));

    cia.writeRegister(0x00, 0x10);

    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));

    cia.writeRegister(0x00, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
}

void C64CIA2PortTest::testDataOutput()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    //
    // PA5 = DATA OUT.
    //
    cia.writeRegister(0x00, 0x00);
    cia.writeRegister(0x02, 0x20);

    QVERIFY(bus.lineHigh(IECBus::Line::DATA));

    cia.writeRegister(0x00, 0x20);

    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));

    cia.writeRegister(0x00, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
}

void C64CIA2PortTest::testAttentionOutput()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    //
    // PA6 = ATN OUT.
    //
    cia.writeRegister(0x00, 0x00);
    cia.writeRegister(0x02, 0x40);

    QVERIFY(bus.lineHigh(IECBus::Line::ATN));

    cia.writeRegister(0x00, 0x40);

    QVERIFY(!bus.lineHigh(IECBus::Line::ATN));

    cia.writeRegister(0x00, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
}

void C64CIA2PortTest::testClockInput()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    const IECBus::ParticipantId drive = bus.addParticipant();

    //
    // PA2 = CLK IN.
    //
    cia.writeRegister(0x02, 0x00);

    QVERIFY((cia.readRegister(0x00) & 0x04) != 0);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::CLK, true));

    QVERIFY((cia.readRegister(0x00) & 0x04) == 0);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::CLK, false));

    QVERIFY((cia.readRegister(0x00) & 0x04) != 0);
}

void C64CIA2PortTest::testDataInput()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    const IECBus::ParticipantId drive = bus.addParticipant();

    //
    // PA3 = DATA IN.
    //
    cia.writeRegister(0x02, 0x00);

    QVERIFY((cia.readRegister(0x00) & 0x08) != 0);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::DATA, true));

    QVERIFY((cia.readRegister(0x00) & 0x08) == 0);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::DATA, false));

    QVERIFY((cia.readRegister(0x00) & 0x08) != 0);
}

void C64CIA2PortTest::testDataDirectionRegister()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    //
    // PRA latch high, but PA4 is initially an input.
    //
    cia.writeRegister(0x00, 0x10);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));

    //
    // PA4 becomes an output and pulls CLK low.
    //
    cia.writeRegister(0x02, 0x10);

    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));

    //
    // Returning PA4 to input releases CLK.
    //
    cia.writeRegister(0x02, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
}

void C64CIA2PortTest::testIndependentIECSignals()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    //
    // PA4, PA5 and PA6 are outputs.
    //
    cia.writeRegister(0x00, 0x00);
    cia.writeRegister(0x02, 0x70);

    cia.writeRegister(0x00, 0x10);

    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
    QVERIFY(bus.lineHigh(IECBus::Line::ATN));

    cia.writeRegister(0x00, 0x60);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));
    QVERIFY(!bus.lineHigh(IECBus::Line::ATN));

    cia.writeRegister(0x00, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
    QVERIFY(bus.lineHigh(IECBus::Line::ATN));
}

void C64CIA2PortTest::testExternalParticipantPullsClockLow()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    const IECBus::ParticipantId drive = bus.addParticipant();

    cia.writeRegister(0x00, 0x00);
    cia.writeRegister(0x02, 0x10);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::CLK, true));

    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));
    QVERIFY((cia.readRegister(0x00) & 0x04) == 0);

    //
    // C64 also pulls CLK low.
    //
    cia.writeRegister(0x00, 0x10);

    //
    // Drive releases CLK, but C64 still holds it.
    //
    QVERIFY(bus.setLineLow(drive, IECBus::Line::CLK, false));

    QVERIFY(!bus.lineHigh(IECBus::Line::CLK));

    cia.writeRegister(0x00, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::CLK));
    QVERIFY((cia.readRegister(0x00) & 0x04) != 0);
}

void C64CIA2PortTest::testExternalParticipantPullsDataLow()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    const IECBus::ParticipantId drive = bus.addParticipant();

    cia.writeRegister(0x00, 0x00);
    cia.writeRegister(0x02, 0x20);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::DATA, true));

    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));
    QVERIFY((cia.readRegister(0x00) & 0x08) == 0);

    cia.writeRegister(0x00, 0x20);

    QVERIFY(bus.setLineLow(drive, IECBus::Line::DATA, false));

    QVERIFY(!bus.lineHigh(IECBus::Line::DATA));

    cia.writeRegister(0x00, 0x00);

    QVERIFY(bus.lineHigh(IECBus::Line::DATA));
    QVERIFY((cia.readRegister(0x00) & 0x08) != 0);
}

void C64CIA2PortTest::testVICBankBitsUnaffected()
{
    IECBus bus;
    C64CIA2Port port;
    port.setIECBus(&bus);
    MOS6526 cia;

    cia.setPort(&port);

    //
    // PA0 and PA1 control the VIC-II memory bank.
    // They must not affect IEC signals.
    //
    cia.writeRegister(0x02, 0x03);

    for (quint8 value = 0; value < 4; ++value)
    {
        cia.writeRegister(0x00, value);

        QCOMPARE(cia.readRegister(0x00) & quint8(0x03), value);

        QVERIFY(bus.lineHigh(IECBus::Line::ATN));
        QVERIFY(bus.lineHigh(IECBus::Line::CLK));
        QVERIFY(bus.lineHigh(IECBus::Line::DATA));
    }
}
