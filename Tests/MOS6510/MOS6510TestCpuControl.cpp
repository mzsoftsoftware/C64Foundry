#include "MOS6510TestCpuControl.h"

#include <QtTest>

MOS6510TestCpuControl::MOS6510TestCpuControl()
{
}

MOS6510TestCpuControl::~MOS6510TestCpuControl()
{
}


void MOS6510TestCpuControl::testReset()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);
    m_cpu.setStackPointer(0x80);
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0xFFFC, 0x56);
    m_memory.writeRAM(0xFFFD, 0x34);

    m_cpu.reset();

    //
    // RESET takes seven cycles.
    //
    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // RESET does not initialize A, X or Y.
    //
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));

    //
    // Interrupts are disabled after RESET.
    //
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));
}

void MOS6510TestCpuControl::testResetVector()
{
    setupCpu();

    m_cpu.setProgramCounter(0x4711);
    m_cpu.setStackPointer(0xFF);

    m_memory.writeRAM(0xFFFC, 0xCD);
    m_memory.writeRAM(0xFFFD, 0xAB);

    m_cpu.reset();

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCD));

    //
    // The next cycle must fetch the first opcode at the
    // address loaded from the RESET vector.
    //
    m_memory.writeRAM(0xABCD, 0xEA);

    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCE));
}

void MOS6510TestCpuControl::testResetStackPointer()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0x01);

    m_memory.writeRAM(0x0101, 0xA1);
    m_memory.writeRAM(0x0100, 0xB2);
    m_memory.writeRAM(0x01FF, 0xC3);

    m_memory.writeRAM(0xFFFC, 0x00);
    m_memory.writeRAM(0xFFFD, 0x20);

    m_cpu.reset();

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));

    //
    // RESET performs reads instead of the three writes
    // performed by BRK/IRQ/NMI.
    //
    QCOMPARE(m_memory.readRAM(0x0101), quint8(0xA1));
    QCOMPARE(m_memory.readRAM(0x0100), quint8(0xB2));
    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0xC3));
}

void MOS6510TestCpuControl::testResetStatus()
{
    setupCpu();

    m_cpu.setStatus(0x00);

    m_memory.writeRAM(0xFFFC, 0x00);
    m_memory.writeRAM(0xFFFD, 0x20);

    m_cpu.reset();

    for (int i = 0; i < 7; ++i)
        clock();

    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));
}

void MOS6510TestCpuControl::testResetCycles()
{
    setupCpu();
    m_cpu.setProgramCounter(0x3456);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x3456, 0x11);

    m_memory.writeRAM(0x0180, 0x22);
    m_memory.writeRAM(0x017F, 0x33);
    m_memory.writeRAM(0x017E, 0x44);

    m_memory.writeRAM(0xFFFC, 0xCD);
    m_memory.writeRAM(0xFFFD, 0xAB);

    m_memory.writeRAM(0xABCD, 0xEA);

    m_cpu.reset();

    //
    // C1
    // Dummy read from the current program counter.
    //
    clock();

    verifyRead(0x3456, 0x11);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));

    //
    // C2
    // Second dummy read from the current program counter.
    //
    clock();

    verifyRead(0x3456, 0x11);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));

    //
    // C3
    // Suppressed push of PCH.
    //
    clock();

    verifyRead(0x0180, 0x22);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7F));

    // C4
    clock();
    verifyRead(0x017F, 0x33);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C5
    clock();
    verifyRead(0x017E, 0x44);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    //
    // C6
    // RESET vector low byte.
    //
    clock();

    verifyRead(0xFFFC, 0xCD);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // C7
    // RESET vector high byte.
    //
    clock();

    verifyRead(0xFFFD, 0xAB);

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCD));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    //
    // C8
    // First normal opcode fetch after RESET.
    //
    clock();

    verifyRead(0xABCD, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0xABCE));
}


void MOS6510TestCpuControl::testIrq()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x1234, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x78);
    m_memory.writeRAM(0xFFFF, 0x56);

    m_cpu.setIrqLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x5678));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));
}
void MOS6510TestCpuControl::testIrqMasked()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);
    m_cpu.setStatus(0x24); // I = 1

    m_memory.writeRAM(0x1234, 0xEA); // NOP
    m_memory.writeRAM(0x1235, 0xEA); // NOP

    m_memory.writeRAM(0xFFFE, 0x78);
    m_memory.writeRAM(0xFFFF, 0x56);

    m_cpu.setIrqLine(true);

    clock();
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1235));
}
void MOS6510TestCpuControl::testIrqStack()
{
    setupCpu();

    m_cpu.setProgramCounter(0x3456);
    m_cpu.setStackPointer(0x80);

    // B absichtlich gesetzt, I gelöscht.
    m_cpu.setStatus(0x39);

    m_memory.writeRAM(0x3456, 0xEA);

    m_memory.writeRAM(0xFFFE, 0xCD);
    m_memory.writeRAM(0xFFFF, 0xAB);

    m_cpu.setIrqLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_memory.readRAM(0x0180), quint8(0x34));
    QCOMPARE(m_memory.readRAM(0x017F), quint8(0x56));

    // IRQ: B=0, U=1
    QCOMPARE(m_memory.readRAM(0x017E), quint8(0x29));

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
}
void MOS6510TestCpuControl::testIrqStatus()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);
    m_cpu.setStackPointer(0x80);

    // N, D, Z, C gesetzt; I gelöscht.
    m_cpu.setStatus(0xAB);

    m_memory.writeRAM(0x1234, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x78);
    m_memory.writeRAM(0xFFFF, 0x56);

    m_cpu.setIrqLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::Carry));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::Zero));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::Decimal));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::Negative));

    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));
}
void MOS6510TestCpuControl::testIrqCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x3456);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x39); // B=1, I=0

    m_memory.writeRAM(0x3456, 0xEA);

    m_memory.writeRAM(0xFFFE, 0xCD);
    m_memory.writeRAM(0xFFFF, 0xAB);

    m_memory.writeRAM(0xABCD, 0xEA);

    m_cpu.setIrqLine(true);

    // C1: Dummy opcode read
    clock();
    verifyRead(0x3456, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C2: Second dummy read
    clock();
    verifyRead(0x3456, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C3: Push PCH
    clock();
    verifyWrite(0x0180, 0x34);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7F));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C4: Push PCL
    clock();
    verifyWrite(0x017F, 0x56);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C5: Push status.
    // IRQ pushes B=0 and U=1.
    // I is still clear during this cycle.
    clock();
    verifyWrite(0x017E, 0x29);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C6: Read IRQ vector low and set I
    clock();
    verifyRead(0xFFFE, 0xCD);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C7: Read IRQ vector high and finish interrupt sequence
    clock();
    verifyRead(0xFFFF, 0xAB);

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCD));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C8: First normal opcode fetch in IRQ handler
    clock();
    verifyRead(0xABCD, 0xEA);
}

void MOS6510TestCpuControl::testIrqAfterCli()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStatus(0x24); // I = 1

    m_memory.writeRAM(0x2000, 0x58); // CLI
    m_memory.writeRAM(0x2001, 0xEA); // NOP
    m_memory.writeRAM(0x2002, 0xEA); // must be interrupted

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    m_cpu.setIrqLine(true);

    // CLI
    clock();
    verifyRead(0x2000, 0x58);

    clock();
    verifyRead(0x2001, 0xEA);

    QVERIFY(!m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));

    // IRQ must NOT start here.
    // The following NOP is still executed.
    clock();
    verifyRead(0x2001, 0xEA);

    clock();
    verifyRead(0x2002, 0xEA);

    // Now IRQ starts.
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));
}
void MOS6510TestCpuControl::testIrqAfterSei()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStatus(0x20); // I = 0

    m_memory.writeRAM(0x2000, 0x78); // SEI
    m_memory.writeRAM(0x2001, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    // Fetch SEI while IRQ is not yet active.
    clock();
    verifyRead(0x2000, 0x78);

    // IRQ becomes active during SEI.
    m_cpu.setIrqLine(true);

    // Finish SEI.
    clock();
    verifyRead(0x2001, 0xEA);

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));

    // Nevertheless the IRQ was recognized with the old I state.
    clock();
    verifyRead(0x2001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2001));
}
void MOS6510TestCpuControl::testIrqAfterPlp()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x7F);
    m_cpu.setStatus(0x24); // U=1, I=1

    m_memory.writeRAM(0x2000, 0x28); // PLP
    m_memory.writeRAM(0x2001, 0xEA); // NOP
    m_memory.writeRAM(0x2002, 0xEA);

    // PLP dummy stack read
    m_memory.writeRAM(0x017F, 0xA5);

    // PLP pulls status from $0180:
    // U=1, I=0
    m_memory.writeRAM(0x0180, 0x20);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    m_cpu.setIrqLine(true);

    // --------------------------------------------------------
    // PLP
    // --------------------------------------------------------

    // C1: Opcode fetch
    clock();
    verifyRead(0x2000, 0x28);

    // C2: Dummy read from PC
    clock();
    verifyRead(0x2001, 0xEA);

    // C3: Dummy read from old stack position
    clock();
    verifyRead(0x017F, 0xA5);

    // C4: Pull status
    clock();
    verifyRead(0x0180, 0x20);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));

    QVERIFY(!m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));

    // --------------------------------------------------------
    // Because PLP's change of I is not seen by the IRQ poll
    // belonging to PLP, one more instruction must execute.
    //
    // NOP at $2001
    // --------------------------------------------------------

    // NOP C1: Opcode fetch
    clock();
    verifyRead(0x2001, 0xEA);

    // NOP C2: Implied dummy read
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));

    // --------------------------------------------------------
    // IRQ starts now.
    // --------------------------------------------------------

    // IRQ C1
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));
}
void MOS6510TestCpuControl::testIrqAfterRti()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x7D);
    m_cpu.setStatus(0x24); // I = 1

    m_memory.writeRAM(0x2000, 0x40); // RTI
    m_memory.writeRAM(0x2001, 0xEA);

    m_memory.writeRAM(0x017D, 0xA5);
    m_memory.writeRAM(0x017E, 0x20); // P: I = 0
    m_memory.writeRAM(0x017F, 0x00); // PCL
    m_memory.writeRAM(0x0180, 0x30); // PCH

    m_memory.writeRAM(0x3000, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    m_cpu.setIrqLine(true);

    // RTI C1
    clock();
    verifyRead(0x2000, 0x40);

    // RTI C2
    clock();
    verifyRead(0x2001, 0xEA);

    // RTI C3
    clock();
    verifyRead(0x017D, 0xA5);

    // RTI C4: P
    clock();
    verifyRead(0x017E, 0x20);

    QVERIFY(!m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));

    // RTI C5: PCL
    clock();
    verifyRead(0x017F, 0x00);

    // RTI C6: PCH
    clock();
    verifyRead(0x0180, 0x30);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3000));

    // No NOP at $3000 may execute.
    // IRQ starts immediately.
    clock();
    verifyRead(0x3000, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3000));

    // IRQ C2
    clock();
    verifyRead(0x3000, 0xEA);
}void MOS6510TestCpuControl::testIrqReleased()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);
    m_memory.writeRAM(0x2001, 0xEA);

    m_cpu.setIrqLine(true);
    m_cpu.setIrqLine(false);

    clock();
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x2001));
}

void MOS6510TestCpuControl::testIrqBranchNotTaken()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);

    // U=1, Z=1, I=0
    // BNE is therefore not taken.
    m_cpu.setStatus(0x22);

    m_memory.writeRAM(0x2000, 0xD0); // BNE
    m_memory.writeRAM(0x2001, 0x10); // Offset
    m_memory.writeRAM(0x2002, 0xEA); // Next opcode

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    // Branch C1: opcode fetch.
    clock();
    verifyRead(0x2000, 0xD0);

    // IRQ becomes active before the branch interrupt poll.
    m_cpu.setIrqLine(true);

    // Branch C2: operand fetch.
    clock();
    verifyRead(0x2001, 0x10);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));

    // IRQ C1.
    // The opcode at $2002 must not execute.
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));

    // IRQ C2.
    clock();
    verifyRead(0x2002, 0xEA);
}
void MOS6510TestCpuControl::testIrqBranchTaken()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);

    // U=1, Z=0, I=0
    // BNE is taken.
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xD0); // BNE
    m_memory.writeRAM(0x2001, 0x10); // Target = $2012

    m_memory.writeRAM(0x2002, 0xEA); // Branch dummy read
    m_memory.writeRAM(0x2012, 0xEA); // NOP at branch target
    m_memory.writeRAM(0x2013, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    // Branch C1: opcode fetch.
    clock();
    verifyRead(0x2000, 0xD0);

    // Branch C2: operand fetch and interrupt poll.
    // IRQ is not active yet.
    clock();
    verifyRead(0x2001, 0x10);

    // IRQ becomes active AFTER the branch poll.
    m_cpu.setIrqLine(true);

    // Branch C3:
    // Taken branch dummy read.
    // No additional interrupt poll occurs here.
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2012));

    // Therefore the instruction at the branch target must
    // still execute.

    // NOP C1.
    clock();
    verifyRead(0x2012, 0xEA);

    // NOP C2.
    clock();
    verifyRead(0x2013, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2013));

    // IRQ starts only now.
    clock();
    verifyRead(0x2013, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2013));
}
void MOS6510TestCpuControl::testIrqBranchTakenPageCrossing()
{
    setupCpu();

    m_cpu.setProgramCounter(0x20FD);

    // U=1, Z=0, I=0
    // BNE is taken.
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x20FD, 0xD0); // BNE
    m_memory.writeRAM(0x20FE, 0x02); // Target = $2101

    // C3 dummy read.
    m_memory.writeRAM(0x20FF, 0xEA);

    // C4 wrong-page dummy read.
    m_memory.writeRAM(0x2001, 0xEA);

    // Branch target.
    m_memory.writeRAM(0x2101, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x40);

    // Branch C1: opcode fetch.
    clock();
    verifyRead(0x20FD, 0xD0);

    // Branch C2: operand fetch and first interrupt poll.
    // IRQ is not active yet.
    clock();
    verifyRead(0x20FE, 0x02);

    // IRQ becomes active after the first poll.
    m_cpu.setIrqLine(true);

    // Branch C3:
    // Taken branch dummy read.
    clock();
    verifyRead(0x20FF, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2101));

    // Branch C4:
    // Page-crossing dummy read.
    //
    // The additional interrupt polling opportunity caused
    // by the page crossing must recognize the IRQ.
    clock();
    verifyRead(0x2001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2101));

    // IRQ C1.
    // Opcode at the branch target must NOT execute.
    clock();
    verifyRead(0x2101, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2101));

    // IRQ C2.
    clock();
    verifyRead(0x2101, 0xEA);
}
