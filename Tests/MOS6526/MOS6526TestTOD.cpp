#include "MOS6526TestTOD.h"

#include <QTest>

#include "C64/CIA/MOS6526.h"


MOS6526TestTOD::MOS6526TestTOD()
{
}

MOS6526TestTOD::~MOS6526TestTOD()
{
}


void MOS6526TestTOD::testTODInitialState()
{
    MOS6526 cia;

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));
    QCOMPARE(cia.readRegister(0x09), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0A), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0B), quint8(0x00));
}


void MOS6526TestTOD::testTODTenthsDivider()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x00);

    for (int i = 0; i < 4; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x01));

    for (int i = 0; i < 45; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));
    QCOMPARE(cia.readRegister(0x09), quint8(0x01));
}


void MOS6526TestTOD::testTODSecondsAndMinutesRollover()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x59);
    cia.writeRegister(0x09, 0x59);
    cia.writeRegister(0x08, 0x09);

    for (int i = 0; i < 5; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));
    QCOMPARE(cia.readRegister(0x09), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0A), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0B), quint8(0x02));
}


void MOS6526TestTOD::testTODHoursRollover()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    // 11:59:59.9 AM.
    cia.writeRegister(0x0B, 0x11);
    cia.writeRegister(0x0A, 0x59);
    cia.writeRegister(0x09, 0x59);
    cia.writeRegister(0x08, 0x09);

    for (int i = 0; i < 5; ++i)
        cia.clockTODPulse();

    // 12:00:00.0 PM.
    QCOMPARE(cia.readRegister(0x08), quint8(0x00));
    QCOMPARE(cia.readRegister(0x09), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0A), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0B), quint8(0x92));

    // Release the TOD read latch.
    cia.readRegister(0x08);

    // Advance another twelve hours to 12:00 AM.
    for (int i = 0; i < 12 * 60 * 60 * 10 * 5; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x0B), quint8(0x12));
}


void MOS6526TestTOD::testTODReadLatch()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x02);
    cia.writeRegister(0x09, 0x03);
    cia.writeRegister(0x08, 0x00);

    // Reading hours latches the current time.
    QCOMPARE(cia.readRegister(0x0B), quint8(0x01));

    for (int i = 0; i < 5; ++i)
        cia.clockTODPulse();

    // Other reads still return the latched time.
    QCOMPARE(cia.readRegister(0x0A), quint8(0x02));
    QCOMPARE(cia.readRegister(0x09), quint8(0x03));

    // Reading tenths releases the latch.
    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    for (int i = 0; i < 45; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x09), quint8(0x04));
}


void MOS6526TestTOD::testTODClockStopAndRestart()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    // Writing hours stops the TOD clock.
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x00);
    cia.writeRegister(0x0B, 0x01);

    for (int i = 0; i < 50; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));
    QCOMPARE(cia.readRegister(0x09), quint8(0x00));

    // Writing tenths restarts the TOD clock.
    cia.writeRegister(0x08, 0x00);

    for (int i = 0; i < 5; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x01));
}


void MOS6526TestTOD::testTODAlarmInterrupt()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    // Program the alarm for 01:00:00.0 PM.
    cia.writeRegister(0x0F, 0x80);
    cia.writeRegister(0x0B, 0x81);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x00);

    // Select the TOD clock registers again.
    cia.writeRegister(0x0F, 0x00);

    // Start at 12:59:59.9 PM.
    cia.writeRegister(0x0B, 0x92);
    cia.writeRegister(0x0A, 0x59);
    cia.writeRegister(0x09, 0x59);
    cia.writeRegister(0x08, 0x09);

    cia.writeRegister(0x0D, 0x84);

    for (int i = 0; i < 5; ++i)
        cia.clockTODPulse();

    QVERIFY(cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x84));
    QVERIFY(!cia.irq());
}


void MOS6526TestTOD::testTODSixtyHertzDivider()
{
    MOS6526 cia;

    cia.setTODDivider(6);

    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x00);

    for (int i = 0; i < 5; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x01));
}


void MOS6526TestTOD::testTODRegisterMirroring()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    cia.writeRegister(0x1B, 0x01);
    QCOMPARE(cia.readRegister(0x0B), quint8(0x01));
    QCOMPARE(cia.readRegister(0x2B), quint8(0x01));
}


void MOS6526TestTOD::testTODZeroDividerIsIgnored()
{
    MOS6526 cia;

    //
    // The default divider is five pulses per tenth.
    // A divider value of zero must be ignored.
    //
    cia.setTODDivider(0);

    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x08, 0x00);

    for (int i = 0; i < 4; ++i)
        cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x01));
}


void MOS6526TestTOD::testTODDividerChangeResetsPrescaler()
{
    MOS6526 cia;

    cia.setTODDivider(5);

    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x08, 0x00);

    //
    // Accumulate part of a divider interval.
    //
    cia.clockTODPulse();
    cia.clockTODPulse();
    cia.clockTODPulse();

    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    //
    // Changing the divider resets the prescaler counter.
    //
    cia.setTODDivider(2);

    cia.clockTODPulse();
    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    cia.clockTODPulse();
    QCOMPARE(cia.readRegister(0x08), quint8(0x01));
}


void MOS6526TestTOD::testTODAlarmStatusWithoutInterruptMask()
{
    MOS6526 cia;

    cia.setTODDivider(1);

    //
    // Program the alarm for 01:00:00.1 AM.
    //
    cia.writeRegister(0x0F, 0x80);
    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x01);

    //
    // Select the TOD clock registers and set the time
    // immediately before the alarm value.
    //
    cia.writeRegister(0x0F, 0x00);
    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x00);

    //
    // Leave the TOD alarm interrupt masked.
    //
    cia.clockTODPulse();

    QVERIFY(!cia.irq());

    //
    // ICR status bit 2 is still set even when the source is masked.
    // Bit 7 remains clear because no enabled source is pending.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x04));
}


void MOS6526TestTOD::testTODAlarmDoesNotOverwriteClockRegisters()
{
    MOS6526 cia;

    //
    // Program an alarm without changing the TOD clock.
    //
    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x12);
    cia.writeRegister(0x09, 0x34);
    cia.writeRegister(0x08, 0x05);

    cia.writeRegister(0x0F, 0x80);
    cia.writeRegister(0x0B, 0x82);
    cia.writeRegister(0x0A, 0x56);
    cia.writeRegister(0x09, 0x45);
    cia.writeRegister(0x08, 0x07);

    //
    // Select the TOD clock registers again.
    //
    cia.writeRegister(0x0F, 0x00);

    QCOMPARE(cia.readRegister(0x0B), quint8(0x01));
    QCOMPARE(cia.readRegister(0x0A), quint8(0x12));
    QCOMPARE(cia.readRegister(0x09), quint8(0x34));
    QCOMPARE(cia.readRegister(0x08), quint8(0x05));
}


void MOS6526TestTOD::testTODRegisterValueMasks()
{
    MOS6526 cia;

    //
    // TOD registers accept only the defined BCD and AM/PM bits.
    //
    cia.writeRegister(0x0B, 0xFF);
    cia.writeRegister(0x0A, 0xFF);
    cia.writeRegister(0x09, 0xFF);
    cia.writeRegister(0x08, 0xFF);

    QCOMPARE(cia.readRegister(0x0B), quint8(0x9F));
    QCOMPARE(cia.readRegister(0x0A), quint8(0x7F));
    QCOMPARE(cia.readRegister(0x09), quint8(0x7F));
    QCOMPARE(cia.readRegister(0x08), quint8(0x0F));
}



void MOS6526TestTOD::testTODResetClearsClockAndAlarm()
{
    MOS6526 cia;

    cia.setTODDivider(1);

    //
    // Program an alarm for 01:00:00.1 AM.
    //
    cia.writeRegister(0x0F, 0x80);
    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x01);

    //
    // Reset must clear the clock and the programmed alarm.
    //
    cia.reset();

    QCOMPARE(cia.readRegister(0x0B), quint8(0x00));
    QCOMPARE(cia.readRegister(0x0A), quint8(0x00));
    QCOMPARE(cia.readRegister(0x09), quint8(0x00));
    QCOMPARE(cia.readRegister(0x08), quint8(0x00));

    //
    // Set the clock to the value that matched the old alarm
    // immediately before its expected trigger.
    //
    cia.writeRegister(0x0B, 0x01);
    cia.writeRegister(0x0A, 0x00);
    cia.writeRegister(0x09, 0x00);
    cia.writeRegister(0x08, 0x00);

    cia.clockTODPulse();

    //
    // The old alarm must no longer trigger after reset.
    //
    QVERIFY(!cia.irq());
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));
}
