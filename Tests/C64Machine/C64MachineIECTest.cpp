#include "C64MachineIECTest.h"

#include <QTest>

#include "C64/C64Machine.h"
#include "C64/IEC/IECBus.h"
#include "C64/UserPort/UserPortBus.h"


C64MachineIECTest::C64MachineIECTest()
{
}

C64MachineIECTest::~C64MachineIECTest()
{
}


void C64MachineIECTest::testInitialIECLineStates()
{
    IECBus bus;
    C64Machine machine;
    machine.setIECBus(&bus);
    IECBus* ptrBus = &bus;

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testCIA2IECOutputs()
{
    IECBus bus;
    C64Machine machine;
    machine.setIECBus(&bus);
    IECBus* ptrBus = &bus;
    machine.powerOn();

    //
    // PA3-PA5 are IEC outputs.
    //
    machine.writeCIA2Register(0x00, 0x00);
    machine.writeCIA2Register(0x02, 0x38);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    machine.writeCIA2Register(0x00, 0x38);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    machine.writeCIA2Register(0x00, 0x00);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testCIA2DataDirection()
{
    IECBus bus;
    C64Machine machine;
    machine.setIECBus(&bus);
    IECBus* ptrBus = &bus;
    machine.powerOn();

    // PRA high, but all pins configured as inputs.
    machine.writeCIA2Register(0x00, 0x38);
    machine.writeCIA2Register(0x02, 0x00);

    // All lines are released.
    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    // Enable CLK output.
    machine.writeCIA2Register(0x02, 0x10);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    // Enable all IEC outputs.
    machine.writeCIA2Register(0x02, 0x38);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    // Return all pins to inputs.
    machine.writeCIA2Register(0x02, 0x00);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testExternalIECInputs()
{
    IECBus bus;
    C64Machine machine;
    machine.setIECBus(&bus);
    IECBus* ptrBus = &bus;
    machine.powerOn();

    const IECBus::ParticipantId drive = ptrBus->addParticipant();

    // PA6 = CLK IN, PA7 = DATA IN.
    machine.writeCIA2Register(0x02, 0x00);

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0xC0), quint8(0xC0));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::CLK, true));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0xC0), quint8(0x80));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::DATA, true));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0xC0), quint8(0x00));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::CLK, false));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0xC0), quint8(0x40));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::DATA, false));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0xC0), quint8(0xC0));
}

void C64MachineIECTest::testVICBankBitsUnaffected()
{
    IECBus bus;
    C64Machine machine;
    machine.setIECBus(&bus);
    IECBus* ptrBus = &bus;

    //
    // PA0 and PA1 select the VIC-II memory bank.
    //
    machine.writeCIA2Register(0x02, 0x03);

    for (quint8 value = 0; value < 4; ++value)
    {
        machine.writeCIA2Register(0x00, value);

        QCOMPARE(machine.readCIA2Register(0x00) & quint8(0x03), value);

        QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
        QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
        QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
    }
}

void C64MachineIECTest::testResetAndPowerOn()
{
    IECBus bus;
    C64Machine machine;
    machine.setIECBus(&bus);
    IECBus* ptrBus = &bus;
    machine.powerOn();

    //
    // Pull all IEC lines low.
    //
    machine.writeCIA2Register(0x02, 0x38);
    machine.writeCIA2Register(0x00, 0x38);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    machine.softReset();

    //
    // Reset must release the C64's IEC outputs.
    //
    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Assert the lines again.
    //
    machine.writeCIA2Register(0x02, 0x38);
    machine.writeCIA2Register(0x00, 0x38);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // PowerOff must release all IEC lines.
    //
    machine.powerOff();

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // PowerOn must reconnect the C64 with released IEC lines.
    //
    machine.powerOn();

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testCIA2SerialOutput()
{
    C64Machine machine;
    machine.powerOn();

    // CIA2 Timer A mit dem kürzestmöglichen Intervall starten.
    machine.writeCIA2Register(0x04, 0x00);
    machine.writeCIA2Register(0x05, 0x00);

    // Serial Output Mode und Timer A starten.
    machine.writeCIA2Register(0x0E, 0x41);

    // Erstes auszugebendes Byte: $A5.
    machine.writeCIA2Register(0x0C, 0xA5);

    // Ausgangszustand vor der ersten Timer-A-Unterlauf-Flanke.
    QVERIFY(machine.cia2SerialDataOutput());
    QVERIFY(machine.cia2SerialClockOutput());

    // Erster Timer-A-Unterlauf: MSB von $A5 wird ausgegeben,
    // CNT wechselt auf LOW.
    machine.clock();

    QVERIFY(machine.cia2SerialDataOutput());
    QVERIFY(!machine.cia2SerialClockOutput());

    // Zweiter Timer-A-Unterlauf: CNT wechselt wieder auf HIGH.
    machine.clock();

    QVERIFY(machine.cia2SerialDataOutput());
    QVERIFY(machine.cia2SerialClockOutput());

    // Dritter Timer-A-Unterlauf: zweites Bit von $A5 ist 0.
    machine.clock();

    QVERIFY(!machine.cia2SerialDataOutput());
    QVERIFY(!machine.cia2SerialClockOutput());
}

void C64MachineIECTest::testCIA2SerialOutputReset()
{
    C64Machine machine;
    machine.powerOn();

    machine.writeCIA2Register(0x04, 0x00);
    machine.writeCIA2Register(0x05, 0x00);
    machine.writeCIA2Register(0x0E, 0x41);
    machine.writeCIA2Register(0x0C, 0xA5);

    // Ersten Bit-Takt ausführen.
    machine.clock();

    QVERIFY(machine.cia2SerialDataOutput());
    QVERIFY(!machine.cia2SerialClockOutput());

    // Reset muss beide Ausgänge freigeben.
    machine.softReset();

    QVERIFY(machine.cia2SerialDataOutput());
    QVERIFY(machine.cia2SerialClockOutput());
}

void C64MachineIECTest::testUserPortInitialLineStates()
{
    UserPortBus bus;

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(bus.lineHigh(UserPortBus::Line::CNT));
}

void C64MachineIECTest::testUserPortMultipleParticipants()
{
    UserPortBus bus;

    const UserPortBus::ParticipantId first = bus.addParticipant();
    const UserPortBus::ParticipantId second = bus.addParticipant();

    QVERIFY(bus.setLineLow(first, UserPortBus::Line::SP, true));
    QVERIFY(!bus.lineHigh(UserPortBus::Line::SP));

    // Ein zweiter Teilnehmer zieht CNT ebenfalls auf LOW.
    QVERIFY(bus.setLineLow(second, UserPortBus::Line::CNT, true));

    // Das Freigeben durch Teilnehmer 1 darf SP nicht beeinflussen,
    // solange Teilnehmer 2 SP weiterhin auf LOW ziehen würde.
    QVERIFY(bus.setLineLow(first, UserPortBus::Line::SP, false));
    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(!bus.lineHigh(UserPortBus::Line::CNT));

    // Entfernen von Teilnehmer 2 gibt CNT frei.
    QVERIFY(bus.removeParticipant(second));

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(bus.lineHigh(UserPortBus::Line::CNT));

    QVERIFY(!bus.removeParticipant(second));
}

void C64MachineIECTest::testCIA2SerialOutputToUserPort()
{
    UserPortBus bus;
    C64Machine machine;

    machine.setUserPortBus(&bus);
    machine.powerOn();

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(bus.lineHigh(UserPortBus::Line::CNT));

    machine.writeCIA2Register(0x04, 0x00);
    machine.writeCIA2Register(0x05, 0x00);
    machine.writeCIA2Register(0x0E, 0x41);
    machine.writeCIA2Register(0x0C, 0xA5);

    // Erster Timer-A-Unterlauf: SP = 1, CNT = 0.
    machine.clock();

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(!bus.lineHigh(UserPortBus::Line::CNT));

    // Zweiter Timer-A-Unterlauf: CNT = 1.
    machine.clock();

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(bus.lineHigh(UserPortBus::Line::CNT));

    // Dritter Timer-A-Unterlauf: SP = 0, CNT = 0.
    machine.clock();

    QVERIFY(!bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(!bus.lineHigh(UserPortBus::Line::CNT));

    // Reset gibt beide CIA-Ausgänge frei.
    machine.softReset();

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(bus.lineHigh(UserPortBus::Line::CNT));

    // Ausschalten entfernt den CIA2-Teilnehmer.
    machine.powerOff();

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));
    QVERIFY(bus.lineHigh(UserPortBus::Line::CNT));
}

void C64MachineIECTest::testUserPortExternalDevice()
{
    UserPortBus bus;
    C64Machine machine;

    machine.setUserPortBus(&bus);
    machine.powerOn();

    const UserPortBus::ParticipantId externalDevice = bus.addParticipant();

    // Das externe Gerät zieht SP auf LOW.
    QVERIFY(bus.setLineLow(externalDevice, UserPortBus::Line::SP, true));
    QVERIFY(!bus.lineHigh(UserPortBus::Line::SP));

    // CIA2 setzt SP auf HIGH und gibt seine Leitung frei.
    // Das externe Gerät hält SP weiterhin auf LOW.
    machine.writeCIA2Register(0x04, 0x00);
    machine.writeCIA2Register(0x05, 0x00);
    machine.writeCIA2Register(0x0E, 0x41);
    machine.writeCIA2Register(0x0C, 0xA5);

    machine.clock();
    machine.clock();

    QVERIFY(!bus.lineHigh(UserPortBus::Line::SP));

    // Erst das externe Gerät gibt die Leitung frei.
    QVERIFY(bus.setLineLow(externalDevice, UserPortBus::Line::SP, false));

    QVERIFY(bus.lineHigh(UserPortBus::Line::SP));

    QVERIFY(bus.removeParticipant(externalDevice));
}
