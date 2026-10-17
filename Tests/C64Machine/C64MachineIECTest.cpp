#include "C64MachineIECTest.h"

#include <QTest>

#include "C64/C64Machine.h"
#include "C64/IEC/IECBus.h"


C64MachineIECTest::C64MachineIECTest()
{
}

C64MachineIECTest::~C64MachineIECTest()
{
}


void C64MachineIECTest::testInitialIECLineStates()
{
    C64Machine machine;

    IECBus* ptrBus = machine.iecBus();

    QVERIFY(ptrBus != nullptr);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testCIA2IECOutputs()
{
    C64Machine machine;

    IECBus* ptrBus = machine.iecBus();

    QVERIFY(ptrBus != nullptr);

    //
    // PA4-PA6 are IEC outputs.
    //
    machine.writeCIA2Register(0x00, 0x00);
    machine.writeCIA2Register(0x02, 0x70);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Assert all three IEC lines.
    //
    machine.writeCIA2Register(0x00, 0x70);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Release all three IEC lines.
    //
    machine.writeCIA2Register(0x00, 0x00);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testCIA2DataDirection()
{
    C64Machine machine;

    IECBus* ptrBus = machine.iecBus();

    QVERIFY(ptrBus != nullptr);

    //
    // PRA high, but all pins configured as inputs.
    //
    machine.writeCIA2Register(0x00, 0x70);
    machine.writeCIA2Register(0x02, 0x00);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Enable CLK output.
    //
    machine.writeCIA2Register(0x02, 0x10);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Enable all IEC outputs.
    //
    machine.writeCIA2Register(0x02, 0x70);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Return all pins to inputs.
    //
    machine.writeCIA2Register(0x02, 0x00);

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}

void C64MachineIECTest::testExternalIECInputs()
{
    C64Machine machine;

    IECBus* ptrBus = machine.iecBus();

    QVERIFY(ptrBus != nullptr);

    const IECBus::ParticipantId drive = ptrBus->addParticipant();

    //
    // PA2 = CLK IN, PA3 = DATA IN.
    //
    machine.writeCIA2Register(0x02, 0x00);

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0x0C), quint8(0x0C));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::CLK, true));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0x0C), quint8(0x08));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::DATA, true));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0x0C), quint8(0x00));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::CLK, false));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0x0C), quint8(0x04));

    QVERIFY(ptrBus->setLineLow(drive, IECBus::Line::DATA, false));

    QCOMPARE(machine.readCIA2Register(0x00) & quint8(0x0C), quint8(0x0C));

    QVERIFY(ptrBus->removeParticipant(drive));
}

void C64MachineIECTest::testVICBankBitsUnaffected()
{
    C64Machine machine;

    IECBus* ptrBus = machine.iecBus();

    QVERIFY(ptrBus != nullptr);

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
    C64Machine machine;

    IECBus* ptrBus = machine.iecBus();

    QVERIFY(ptrBus != nullptr);

    //
    // Pull all IEC lines low.
    //
    machine.writeCIA2Register(0x02, 0x70);
    machine.writeCIA2Register(0x00, 0x70);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(!ptrBus->lineHigh(IECBus::Line::DATA));

    machine.reset();

    //
    // Reset must release the C64's IEC outputs.
    //
    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));

    //
    // Assert the lines again.
    //
    machine.writeCIA2Register(0x02, 0x70);
    machine.writeCIA2Register(0x00, 0x70);

    QVERIFY(!ptrBus->lineHigh(IECBus::Line::ATN));

    machine.powerOn();

    QVERIFY(ptrBus->lineHigh(IECBus::Line::ATN));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::CLK));
    QVERIFY(ptrBus->lineHigh(IECBus::Line::DATA));
}
