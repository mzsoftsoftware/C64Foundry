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

void MOS6526TestRegister::testSerialPortOutputMode()
{
    MOS6526 cia;

    //
    // CRA bit 6 selects serial output mode.
    //
    cia.writeRegister(0x0E, 0x40);

    QCOMPARE(cia.readRegister(0x0E), quint8(0x40));
}


void MOS6526TestRegister::testSerialPortInputMode()
{
    MOS6526 cia;

    //
    // CRA bit 6 clear selects serial input mode.
    //
    cia.writeRegister(0x0E, 0x40);
    cia.writeRegister(0x0E, 0x00);

    QCOMPARE(cia.readRegister(0x0E), quint8(0x00));
}


void MOS6526TestRegister::testSerialDataRegisterAccessibleInBothModes()
{
    MOS6526 cia;

    //
    // Serial output mode.
    //
    cia.writeRegister(0x0E, 0x40);
    cia.writeRegister(0x0C, 0xA5);

    QCOMPARE(cia.readRegister(0x0C), quint8(0xA5));

    //
    // Serial input mode.
    //
    cia.writeRegister(0x0E, 0x00);
    cia.writeRegister(0x0C, 0x3C);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x3C));
}

void MOS6526TestRegister::testSerialPortInputLevel()
{
    MOS6526 cia;

    cia.setSP(false);
    QCOMPARE(cia.serialPortInput(), false);

    cia.setSP(true);
    QCOMPARE(cia.serialPortInput(), true);
}


void MOS6526TestRegister::testSerialPortOutputResetState()
{
    MOS6526 cia;

    QCOMPARE(cia.serialPortOutput(), true);

    cia.reset();

    QCOMPARE(cia.serialPortOutput(), true);
}

void MOS6526TestRegister::testSerialPortInputShiftsEightBits()
{
    MOS6526 cia;

    //
    // Serial input mode.
    //
    cia.writeRegister(0x0E, 0x00);
    cia.writeRegister(0x0C, 0x00);

    //
    // Transfer $A5, MSB first.
    //
    const quint8 data = 0xA5;

    for (int bit = 7; bit >= 0; --bit)
    {
        cia.setSP((data & (1 << bit)) != 0);

        //
        // One complete CNT pulse: falling edge, then rising edge.
        //
        cia.setCNT(false);
        cia.setCNT(true);
    }

    QCOMPARE(cia.readRegister(0x0C), quint8(0xA5));
}


void MOS6526TestRegister::testSerialPortInputDoesNotInterruptBeforeEightBits()
{
    MOS6526 cia;

    //
    // Enable serial interrupt.
    //
    cia.writeRegister(0x0D, 0x88);

    //
    // Transfer only seven bits.
    //
    const quint8 data = 0xA5;

    for (int bit = 7; bit >= 1; --bit)
    {
        cia.setSP((data & (1 << bit)) != 0);
        cia.setCNT(false);
        cia.setCNT(true);
    }

    QVERIFY(!cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));
}


void MOS6526TestRegister::testSerialPortInputRaisesInterruptAfterEightBits()
{
    MOS6526 cia;

    //
    // Enable serial interrupt.
    //
    cia.writeRegister(0x0D, 0x88);

    //
    // Transfer eight bits.
    //
    const quint8 data = 0xA5;

    for (int bit = 7; bit >= 0; --bit)
    {
        cia.setSP((data & (1 << bit)) != 0);
        cia.setCNT(false);
        cia.setCNT(true);
    }

    QVERIFY(cia.irq());

    //
    // ICR bit 3 indicates the serial interrupt.
    // Bit 7 indicates that an enabled interrupt is pending.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x88));

    //
    // Reading ICR clears the pending interrupt.
    //
    QVERIFY(!cia.irq());
}

void MOS6526TestRegister::testSerialPortInputShiftsOnlyOnRisingCNT()
{
    MOS6526 cia;

    cia.writeRegister(0x0C, 0x00);

    cia.setSP(true);
    cia.setCNT(false);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x00));

    cia.setCNT(true);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x01));

    cia.setCNT(false);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x01));
}

void MOS6526TestRegister::testSerialPortOutputModeDoesNotShiftInput()
{
    MOS6526 cia;

    cia.writeRegister(0x0E, 0x40);
    cia.writeRegister(0x0C, 0xA5);

    cia.setSP(false);
    cia.setCNT(false);
    cia.setCNT(true);

    QCOMPARE(cia.readRegister(0x0C), quint8(0xA5));
}

void MOS6526TestRegister::testSerialPortInputReceivesConsecutiveBytes()
{
    MOS6526 cia;

    const quint8 firstData = 0xA5;
    const quint8 secondData = 0x3C;

    for (int bit = 7; bit >= 0; --bit)
    {
        cia.setSP((firstData & (1 << bit)) != 0);
        cia.setCNT(false);
        cia.setCNT(true);
    }

    QCOMPARE(cia.readRegister(0x0C), firstData);

    for (int bit = 7; bit >= 0; --bit)
    {
        cia.setSP((secondData & (1 << bit)) != 0);
        cia.setCNT(false);
        cia.setCNT(true);
    }

    QCOMPARE(cia.readRegister(0x0C), secondData);
}

void MOS6526TestRegister::testSerialPortInputResetClearsBitCount()
{
    MOS6526 cia;

    //
    // Start with an incomplete transfer.
    //
    cia.setSP(true);
    cia.setCNT(false);
    cia.setCNT(true);

    cia.setSP(false);
    cia.setCNT(false);
    cia.setCNT(true);

    cia.setSP(true);
    cia.setCNT(false);
    cia.setCNT(true);

    cia.reset();

    //
    // Enable the serial interrupt after reset.
    //
    cia.writeRegister(0x0D, 0x88);

    const quint8 data = 0x5A;

    //
    // Seven bits must not complete the previous transfer.
    //
    for (int bit = 7; bit >= 1; --bit)
    {
        cia.setSP((data & (1 << bit)) != 0);
        cia.setCNT(false);
        cia.setCNT(true);
    }

    QVERIFY(!cia.irq());

    //
    // The eighth bit completes the new byte.
    //
    cia.setSP((data & 0x01) != 0);
    cia.setCNT(false);
    cia.setCNT(true);

    QVERIFY(cia.irq());
    QCOMPARE(cia.readRegister(0x0C), data);
}

void MOS6526TestRegister::testSerialPortInputInterruptWithoutMask()
{
    MOS6526 cia;

    const quint8 data = 0x96;

    for (int bit = 7; bit >= 0; --bit)
    {
        cia.setSP((data & (1 << bit)) != 0);
        cia.setCNT(false);
        cia.setCNT(true);
    }

    //
    // The interrupt source flag is set, but its mask is disabled.
    //
    QVERIFY(!cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x08));
}

void MOS6526TestRegister::testSerialPortOutputDoesNotStartOnSDRWrite()
{
    MOS6526 cia;

    cia.writeRegister(0x0E, 0x40);
    cia.writeRegister(0x0C, 0x25);

    QCOMPARE(cia.serialPortOutput(), true);
}

void MOS6526TestRegister::testSerialPortOutputWaitsForTimerAUnderflow()
{
    MOS6526 cia;

    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);

    cia.writeRegister(0x0C, 0x25);

    //
    // Serial output mode, Timer A started.
    //
    cia.writeRegister(0x0E, 0x41);

    QCOMPARE(cia.serialPortOutput(), true);

    //
    // Timer A counts from 1 to 0.
    //
    cia.clock();

    QCOMPARE(cia.serialPortOutput(), true);

    //
    // Timer A underflows.
    // The first serial bit (bit 7 = 0) is output.
    //
    cia.clock();

    QCOMPARE(cia.serialPortOutput(), false);
}

void MOS6526TestRegister::testSerialPortOutputShiftsMSBFirst()
{
    MOS6526 cia;

    const quint8 data = 0x25;

    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);

    cia.writeRegister(0x0C, data);

    //
    // Serial output mode, Timer A started.
    //
    cia.writeRegister(0x0E, 0x41);

    for (int bit = 7; bit >= 0; --bit)
    {
        cia.clock();

        const bool expectedLevel = (data & (1 << bit)) != 0;

        QCOMPARE(cia.serialPortOutput(), expectedLevel);
    }
}

void MOS6526TestRegister::testSerialPortOutputDoesNotShiftOnExternalCNT()
{
    MOS6526 cia;

    cia.writeRegister(0x0C, 0x25);
    cia.writeRegister(0x0E, 0x40);

    cia.setSP(false);
    cia.setCNT(false);
    cia.setCNT(true);

    cia.setSP(true);
    cia.setCNT(false);
    cia.setCNT(true);

    QCOMPARE(cia.readRegister(0x0C), quint8(0x25));
    QCOMPARE(cia.serialPortOutput(), true);
}

void MOS6526TestRegister::testSerialPortOutputDoesNotInterruptBeforeEightBits()
{
    MOS6526 cia;

    cia.writeRegister(0x0D, 0x88);

    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);

    cia.writeRegister(0x0C, 0xA5);
    cia.writeRegister(0x0E, 0x41);

    for (int bit = 0; bit < 7; ++bit)
    {
        cia.clock();
        QVERIFY(!cia.irq());
    }

    QVERIFY(!cia.irq());

    //
    // Timer A has underflowed, but the serial transfer is incomplete.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x01));
}

void MOS6526TestRegister::testSerialPortOutputRaisesInterruptAfterEightBits()
{
    MOS6526 cia;

    cia.writeRegister(0x0D, 0x88);

    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);

    cia.writeRegister(0x0C, 0xA5);
    cia.writeRegister(0x0E, 0x41);

    for (int bit = 0; bit < 8; ++bit)
        cia.clock();

    QVERIFY(cia.irq());

    //
    // Timer A and serial interrupt status bits are set.
    // Bit 7 indicates an enabled interrupt pending.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x89));

    QVERIFY(!cia.irq());
}

void MOS6526TestRegister::testSerialPortOutputInterruptWithoutMask()
{
    MOS6526 cia;

    cia.writeRegister(0x04, 0x00);
    cia.writeRegister(0x05, 0x00);

    cia.writeRegister(0x0C, 0x96);
    cia.writeRegister(0x0E, 0x41);

    for (int bit = 0; bit < 8; ++bit)
        cia.clock();

    QVERIFY(!cia.irq());

    //
    // Timer A and serial interrupt status bits are set.
    // Neither source has an enabled interrupt mask.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x09));
}
