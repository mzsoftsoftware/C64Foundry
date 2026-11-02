#include "MOS6526TestRegister.h"

#include <QTest>

#include "C64/CIA/MOS6526.h"


MOS6526TestRegister::MOS6526TestRegister()
{
}

MOS6526TestRegister::~MOS6526TestRegister()
{
}


void MOS6526TestRegister::testRegisterMirroring()
{
    MOS6526 cia;

    //
    // CIA registers are mirrored every $10 bytes.
    //
    cia.writeRegister(0x12, 0xA5);

    QCOMPARE(cia.readRegister(0x02), quint8(0xA5));
    QCOMPARE(cia.readRegister(0x12), quint8(0xA5));
    QCOMPARE(cia.readRegister(0x22), quint8(0xA5));
    QCOMPARE(cia.readRegister(0xF2), quint8(0xA5));
}


void MOS6526TestRegister::testControlRegisterAWriteRead()
{
    MOS6526 cia;

    cia.writeRegister(0x0E, 0x2D);

    QCOMPARE(cia.readRegister(0x0E), quint8(0x2D));
}


void MOS6526TestRegister::testControlRegisterBWriteRead()
{
    MOS6526 cia;

    cia.writeRegister(0x0F, 0x6D);

    QCOMPARE(cia.readRegister(0x0F), quint8(0x6D));
}


void MOS6526TestRegister::testTimerAForceLoadBitIsNotStored()
{
    MOS6526 cia;

    cia.writeRegister(0x04, 0x34);
    cia.writeRegister(0x05, 0x12);

    cia.writeRegister(0x0E, 0x11);

    //
    // START is stored, but FORCE LOAD is a strobe.
    //
    QCOMPARE(cia.readRegister(0x0E), quint8(0x01));
}


void MOS6526TestRegister::testTimerBForceLoadBitIsNotStored()
{
    MOS6526 cia;

    cia.writeRegister(0x06, 0x78);
    cia.writeRegister(0x07, 0x56);

    cia.writeRegister(0x0F, 0x11);

    //
    // START is stored, but FORCE LOAD is a strobe.
    //
    QCOMPARE(cia.readRegister(0x0F), quint8(0x01));
}


void MOS6526TestRegister::testInterruptMaskSet()
{
    MOS6526 cia;

    //
    // Bit 7 sets the selected interrupt mask bits.
    //
    cia.writeRegister(0x0D, 0x81);

    //
    // Trigger Timer A underflow.
    //
    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);
    cia.clock();

    QVERIFY(cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x81));
    QVERIFY(!cia.irq());
}


void MOS6526TestRegister::testInterruptMaskClear()
{
    MOS6526 cia;

    cia.writeRegister(0x0D, 0x81);

    //
    // Bit 7 clear means clear the selected mask bits.
    //
    cia.writeRegister(0x0D, 0x01);

    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);
    cia.clock();

    QVERIFY(!cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x01));
}


void MOS6526TestRegister::testMultipleInterruptSources()
{
    MOS6526 cia;

    //
    // Enable Timer A and Timer B interrupts.
    //
    cia.writeRegister(0x0D, 0x83);

    //
    // Both timers underflow on their first clock.
    //
    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);

    cia.writeRegister(0x06, 0x00);
    cia.writeRegister(0x07, 0x00);
    cia.writeRegister(0x0F, 0x01);

    cia.clock();

    QVERIFY(cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x83));
    QVERIFY(!cia.irq());

    //
    // Reading ICR clears all pending interrupt status bits.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));
}


void MOS6526TestRegister::testResetClearsRegistersAndInterrupts()
{
    MOS6526 cia;

    cia.writeRegister(0x00, 0xA5);
    cia.writeRegister(0x02, 0xFF);
    cia.writeRegister(0x01, 0x5A);
    cia.writeRegister(0x03, 0xFF);

    cia.writeRegister(0x0D, 0x81);
    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);
    cia.clock();

    QVERIFY(cia.irq());

    cia.reset();

    QCOMPARE(cia.readRegister(0x00), quint8(0xFF));
    QCOMPARE(cia.readRegister(0x01), quint8(0xFF));
    QCOMPARE(cia.readRegister(0x02), quint8(0x00));
    QCOMPARE(cia.readRegister(0x03), quint8(0x00));
    QCOMPARE(cia.readRegister(0x04), quint8(0x00));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));
    QCOMPARE(cia.readRegister(0x06), quint8(0x00));
    QCOMPARE(cia.readRegister(0x07), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0E), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0F), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));

    QVERIFY(!cia.irq());
}


void MOS6526TestRegister::testInitializeResetsCIA()
{
    MOS6526 cia;

    cia.writeRegister(0x02, 0xFF);
    cia.writeRegister(0x03, 0xFF);
    cia.writeRegister(0x0E, 0x01);
    cia.writeRegister(0x0F, 0x01);

    cia.initialize();

    QCOMPARE(cia.readRegister(0x02), quint8(0x00));
    QCOMPARE(cia.readRegister(0x03), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0E), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0F), quint8(0x00));
}


void MOS6526TestRegister::testSerialDataRegisterWriteRead()
{
    MOS6526 cia;

    cia.writeRegister(0x0C, 0xA5);

    QCOMPARE(cia.readRegister(0x0C), quint8(0xA5));

    cia.writeRegister(0x0C, 0x3C);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x3C));
}


void MOS6526TestRegister::testSerialDataRegisterMirroring()
{
    MOS6526 cia;

    cia.writeRegister(0x1C, 0x96);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x96));
    QCOMPARE(cia.readRegister(0x1C), quint8(0x96));
    QCOMPARE(cia.readRegister(0x2C), quint8(0x96));
    QCOMPARE(cia.readRegister(0xFC), quint8(0x96));
}


void MOS6526TestRegister::testSerialDataRegisterReset()
{
    MOS6526 cia;

    cia.writeRegister(0x0C, 0xA5);

    QCOMPARE(cia.readRegister(0x0C), quint8(0xA5));

    cia.reset();

    QCOMPARE(cia.readRegister(0x0C), quint8(0x00));
}
