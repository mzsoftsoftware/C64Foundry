#include "MOS6526TestTimer.h"

#include <QTest>

#include "C64/CIA/MOS6526.h"


MOS6526TestTimer::MOS6526TestTimer()
{
}
MOS6526TestTimer::~MOS6526TestTimer()
{
}

void MOS6526TestTimer::testTimerALatchLoadWhenStopped()
{
    MOS6526 cia;

    //
    // Timer A is stopped after reset.
    // The counter initially reads $0000.
    //
    QCOMPARE(cia.readRegister(0x04), quint8(0x00));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // Writing the low byte changes only the latch.
    // The counter must remain unchanged.
    //
    cia.writeRegister(0x04, 0x95);

    QCOMPARE(cia.readRegister(0x04), quint8(0x00));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // Writing the high byte while Timer A is stopped
    // loads the complete latch into the counter.
    //
    cia.writeRegister(0x05, 0x42);

    QCOMPARE(cia.readRegister(0x04), quint8(0x95));
    QCOMPARE(cia.readRegister(0x05), quint8(0x42));
}

void MOS6526TestTimer::testTimerAForceLoad()
{
    MOS6526 cia;

    //
    // Load the initial Timer A latch and counter.
    //
    cia.writeRegister(0x04, 0x34);
    cia.writeRegister(0x05, 0x12);

    QCOMPARE(cia.readRegister(0x04), quint8(0x34));
    QCOMPARE(cia.readRegister(0x05), quint8(0x12));

    //
    // Change only the low byte of the latch.
    // The counter must remain unchanged.
    //
    cia.writeRegister(0x04, 0x78);

    QCOMPARE(cia.readRegister(0x04), quint8(0x34));
    QCOMPARE(cia.readRegister(0x05), quint8(0x12));

    //
    // CRA bit 4 forces the Timer A latch into the counter.
    //
    cia.writeRegister(0x0E, 0x10);

    QCOMPARE(cia.readRegister(0x04), quint8(0x78));
    QCOMPARE(cia.readRegister(0x05), quint8(0x12));

    //
    // LOAD is a strobe and must always read back as zero.
    //
    QCOMPARE(cia.readRegister(0x0E) & 0x10, quint8(0x00));
}

void MOS6526TestTimer::testTimerAStopped()
{
    MOS6526 cia;

    //
    // Load Timer A with $0002.
    //
    cia.writeRegister(0x04, 0x02);
    cia.writeRegister(0x05, 0x00);

    QCOMPARE(cia.readRegister(0x04), quint8(0x02));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // Timer A is stopped after reset.
    // Clocking the CIA must not change the counter.
    //
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x02));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x02));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));
}
void MOS6526TestTimer::testTimerACount()
{
    MOS6526 cia;

    //
    // Load Timer A with $0002.
    //
    cia.writeRegister(0x04, 0x02);
    cia.writeRegister(0x05, 0x00);

    QCOMPARE(cia.readRegister(0x04), quint8(0x02));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // Start Timer A.
    //
    cia.writeRegister(0x0E, 0x01);

    QCOMPARE(cia.readRegister(0x0E) & 0x01, quint8(0x01));

    //
    // Timer A counts down once per CIA clock.
    //
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x01));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x00));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));
}

void MOS6526TestTimer::testTimerAUnderflowReload()
{
    MOS6526 cia;

    //
    // Load Timer A with $0002.
    //
    cia.writeRegister(0x04, 0x02);
    cia.writeRegister(0x05, 0x00);

    //
    // Start Timer A in continuous mode.
    //
    cia.writeRegister(0x0E, 0x01);

    //
    // Count down to zero.
    //
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x01));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x00));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // The next clock causes an underflow.
    // In continuous mode the latch is reloaded.
    //
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x02));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));
}

void MOS6526TestTimer::testTimerAInterruptMask()
{
    MOS6526 cia;

    //
    // Enable Timer A interrupts.
    //
    cia.writeRegister(0x0D, 0x81);

    //
    // Reading ICR before an interrupt must return zero.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));

    //
    // Load and start Timer A.
    //
    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);

    //
    // Count down to zero.
    //
    cia.clock();
    QCOMPARE(cia.readRegister(0x04), quint8(0x00));

    //
    // The next clock causes Timer A underflow.
    //
    cia.clock();

    //
    // ICR bit 0 reports the Timer A interrupt.
    // Bit 7 reports that an enabled interrupt occurred.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x81));
}

void MOS6526TestTimer::testInterruptControlRegisterReadClearsStatus()
{
    MOS6526 cia;

    //
    // Enable Timer A interrupts.
    //
    cia.writeRegister(0x0D, 0x81);

    //
    // Load and start Timer A.
    //
    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);

    //
    // Count down to zero and cause an underflow.
    //
    cia.clock();
    cia.clock();

    //
    // The first ICR read returns the pending
    // Timer A interrupt.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x81));

    //
    // Reading the ICR clears the interrupt status.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));
}

void MOS6526TestTimer::testTimerAIrq()
{
    MOS6526 cia;

    //
    // IRQ is inactive after reset.
    //
    QVERIFY(!cia.irq());

    //
    // Enable Timer A interrupts.
    //
    cia.writeRegister(0x0D, 0x81);

    //
    // Load and start Timer A.
    //
    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);

    //
    // Count down to zero and cause an underflow.
    //
    cia.clock();
    cia.clock();

    //
    // Timer A interrupt is pending and enabled.
    //
    QVERIFY(cia.irq());

    //
    // Reading the ICR clears the interrupt status
    // and therefore releases IRQ.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x81));
    QVERIFY(!cia.irq());
}

void MOS6526TestTimer::testTimerAInterruptWithoutMask()
{
    MOS6526 cia;

    //
    // Timer A interrupt mask remains disabled.
    //
    QVERIFY(!cia.irq());

    //
    // Load and start Timer A.
    //
    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);

    //
    // Count down to zero and cause an underflow.
    //
    cia.clock();
    cia.clock();

    //
    // The Timer A interrupt status is set,
    // but IRQ remains inactive because the
    // interrupt source is masked.
    //
    QVERIFY(!cia.irq());

    //
    // ICR reports the Timer A interrupt source,
    // but bit 7 remains clear.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x01));

    //
    // Reading the ICR clears the interrupt status.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));
    QVERIFY(!cia.irq());
}

void MOS6526TestTimer::testInterruptMaskClear()
{
    MOS6526 cia;

    //
    // Enable Timer A interrupts.
    //
    cia.writeRegister(0x0D, 0x81);

    //
    // Disable all interrupt sources.
    // Bit 7 = 0 means clear the specified mask bits.
    //
    cia.writeRegister(0x0D, 0x7F);

    //
    // Load and start Timer A.
    //
    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);
    cia.writeRegister(0x0E, 0x01);

    //
    // Count down to zero and cause an underflow.
    //
    cia.clock();
    cia.clock();

    //
    // Timer A generated an interrupt event,
    // but its interrupt mask was cleared.
    //
    QVERIFY(!cia.irq());

    //
    // The interrupt status is still reported,
    // but bit 7 remains clear.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x01));

    //
    // Reading the ICR clears the status.
    //
    QCOMPARE(cia.readRegister(0x0D), quint8(0x00));
}

void MOS6526TestTimer::testTimerALatchWriteWhileRunning()
{
    MOS6526 cia;

    //
    // Load Timer A with $1234.
    //
    cia.writeRegister(0x04, 0x34);
    cia.writeRegister(0x05, 0x12);

    QCOMPARE(cia.readRegister(0x04), quint8(0x34));
    QCOMPARE(cia.readRegister(0x05), quint8(0x12));

    //
    // Start Timer A.
    //
    cia.writeRegister(0x0E, 0x01);

    //
    // Let the counter decrement once.
    //
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x33));
    QCOMPARE(cia.readRegister(0x05), quint8(0x12));

    //
    // Change the latch to $5678 while Timer A is running.
    //
    cia.writeRegister(0x04, 0x78);
    cia.writeRegister(0x05, 0x56);

    //
    // Writing the latch while the timer is running
    // must not change the current counter.
    //
    QCOMPARE(cia.readRegister(0x04), quint8(0x33));
    QCOMPARE(cia.readRegister(0x05), quint8(0x12));
}

void MOS6526TestTimer::testTimerAOneShot()
{
    MOS6526 cia;

    //
    // Load Timer A with $0001.
    //
    cia.writeRegister(0x04, 0x01);
    cia.writeRegister(0x05, 0x00);

    //
    // Start Timer A in one-shot mode.
    //
    // Bit 0: START
    // Bit 3: RUNMODE (one-shot)
    //
    cia.writeRegister(0x0E, 0x09);

    //
    // Count down to zero.
    //
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x00));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // The next clock causes the underflow.
    //
    cia.clock();

    //
    // The latch is reloaded after underflow.
    //
    QCOMPARE(cia.readRegister(0x04), quint8(0x01));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));

    //
    // One-shot mode automatically clears START.
    //
    QCOMPARE(cia.readRegister(0x0E) & 0x01, quint8(0x00));

    //
    // Further clocks must not decrement Timer A.
    //
    cia.clock();
    cia.clock();

    QCOMPARE(cia.readRegister(0x04), quint8(0x01));
    QCOMPARE(cia.readRegister(0x05), quint8(0x00));
}

void MOS6526TestTimer::testControlRegisterB()
{
    MOS6526 cia;

    //
    // Control Register B is cleared after reset.
    //
    QCOMPARE(cia.readRegister(0x0F), quint8(0x00));

    //
    // Store Timer B control bits.
    //
    cia.writeRegister(0x0F, 0x08);
    QCOMPARE(cia.readRegister(0x0F), quint8(0x08));
}
