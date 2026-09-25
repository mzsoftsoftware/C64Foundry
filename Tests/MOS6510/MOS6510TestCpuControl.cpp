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


void MOS6510TestCpuControl::testNmi()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x1234, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x78);
    m_memory.writeRAM(0xFFFB, 0x56);

    m_cpu.setNmiLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x5678));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));
}
void MOS6510TestCpuControl::testNmiIgnoredInterruptDisable()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);

    // I = 1
    m_cpu.setStatus(0x24);

    m_memory.writeRAM(0x1234, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x78);
    m_memory.writeRAM(0xFFFB, 0x56);

    m_cpu.setNmiLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    //
    // NMI is not masked by I.
    //
    QCOMPARE(m_cpu.programCounter(), quint16(0x5678));
}
void MOS6510TestCpuControl::testNmiStack()
{
    setupCpu();

    m_cpu.setProgramCounter(0x3456);
    m_cpu.setStackPointer(0x80);

    // B deliberately set.
    m_cpu.setStatus(0x39);

    m_memory.writeRAM(0x3456, 0xEA);

    m_memory.writeRAM(0xFFFA, 0xCD);
    m_memory.writeRAM(0xFFFB, 0xAB);

    m_cpu.setNmiLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_memory.readRAM(0x0180), quint8(0x34));
    QCOMPARE(m_memory.readRAM(0x017F), quint8(0x56));

    //
    // NMI pushes B=0 and U=1.
    //
    QCOMPARE(m_memory.readRAM(0x017E), quint8(0x29));

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
}
void MOS6510TestCpuControl::testNmiStatus()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1234);
    m_cpu.setStackPointer(0x80);

    //
    // N, D, Z, C set; I clear.
    //
    m_cpu.setStatus(0xAB);

    m_memory.writeRAM(0x1234, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x78);
    m_memory.writeRAM(0xFFFB, 0x56);

    m_cpu.setNmiLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::Carry));

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::Zero));

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::Decimal));

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::Negative));

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));
}
void MOS6510TestCpuControl::testNmiCycles()
{
    setupCpu();

    m_cpu.setProgramCounter(0x3456);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x39); // B=1, I=0

    m_memory.writeRAM(0x3456, 0xEA);

    m_memory.writeRAM(0xFFFA, 0xCD);
    m_memory.writeRAM(0xFFFB, 0xAB);

    m_memory.writeRAM(0xABCD, 0xEA);

    m_cpu.setNmiLine(true);

    //
    // C1
    // Suppressed opcode fetch.
    //
    clock();

    verifyRead(0x3456, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));

    //
    // C2
    // Second dummy read.
    //
    clock();

    verifyRead(0x3456, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));

    //
    // C3
    // Push PCH.
    //
    clock();

    verifyWrite(0x0180, 0x34);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7F));

    //
    // C4
    // Push PCL.
    //
    clock();

    verifyWrite(0x017F, 0x56);
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));

    //
    // C5
    // Push P with B=0 and U=1.
    //
    clock();

    verifyWrite(0x017E, 0x29);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    QVERIFY(!m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));

    //
    // C6
    // Set I and read NMI vector low.
    //
    clock();

    verifyRead(0xFFFA, 0xCD);

    QVERIFY(m_cpu.statusFlag(
        MOS6510StatusFlag::InterruptDisable));

    //
    // C7
    // Read NMI vector high.
    //
    clock();

    verifyRead(0xFFFB, 0xAB);

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCD));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // C8
    // First normal opcode fetch in NMI handler.
    //
    clock();

    verifyRead(0xABCD, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCE));
}
void MOS6510TestCpuControl::testNmiEdgeTriggered()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    m_memory.writeRAM(0x4000, 0xEA);
    m_memory.writeRAM(0x4001, 0xEA);

    //
    // Generate one NMI edge and leave the line active.
    //
    m_cpu.setNmiLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // NMI line is still active.
    //
    QVERIFY(m_cpu.nmiLine());

    //
    // Nevertheless the first handler instruction must
    // execute normally. A level-sensitive implementation
    // would incorrectly start another NMI here.
    //
    clock();
    verifyRead(0x4000, 0xEA);

    clock();
    verifyRead(0x4001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
}
void MOS6510TestCpuControl::testNmiSecondEdge()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    m_memory.writeRAM(0x4000, 0xEA);
    m_memory.writeRAM(0x4001, 0xEA);

    //
    // First NMI edge.
    //
    m_cpu.setNmiLine(true);

    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // Release the NMI line and generate a second edge.
    //
    m_cpu.setNmiLine(false);
    m_cpu.setNmiLine(true);

    //
    // The interrupt sequence itself does not poll for
    // another interrupt. Therefore the first handler
    // instruction must execute before the second NMI
    // can start.
    //
    clock();
    verifyRead(0x4000, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4001));

    clock();
    verifyRead(0x4001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4001));

    //
    // Now the second NMI starts.
    //
    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));

    //
    // Three more bytes were pushed.
    //
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7A));
}
void MOS6510TestCpuControl::testNmiDuringInstruction()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    //
    // LDA $1234
    //
    m_memory.writeRAM(0x2000, 0xAD);
    m_memory.writeRAM(0x2001, 0x34);
    m_memory.writeRAM(0x2002, 0x12);
    m_memory.writeRAM(0x2003, 0xEA);
    m_memory.writeRAM(0x1234, 0x42);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // C1: Opcode fetch.
    //
    clock();
    verifyRead(0x2000, 0xAD);

    //
    // NMI edge while the instruction is running.
    //
    m_cpu.setNmiLine(true);

    //
    // C2: Address low.
    //
    clock();
    verifyRead(0x2001, 0x34);

    //
    // C3: Address high.
    //
    clock();
    verifyRead(0x2002, 0x12);

    //
    // C4: Operand.
    //
    clock();
    verifyRead(0x1234, 0x42);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.programCounter(), quint16(0x2003));

    //
    // The following cycle must start NMI, not fetch
    // the instruction at $2003.
    //
    clock();
    verifyRead(0x2003, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2003));
}
void MOS6510TestCpuControl::testNmiDuringBranchPageCrossing()
{
    setupCpu();

    m_cpu.setProgramCounter(0x20FD);
    m_cpu.setStackPointer(0x80);

    //
    // Z = 0 -> BNE taken.
    //
    m_cpu.setStatus(0x20);

    //
    // $20FD: BNE +2
    // Base after operand = $20FF
    // Target             = $2101
    //
    m_memory.writeRAM(0x20FD, 0xD0);
    m_memory.writeRAM(0x20FE, 0x02);
    m_memory.writeRAM(0x20FF, 0xEA);
    m_memory.writeRAM(0x2001, 0xEA);
    m_memory.writeRAM(0x2101, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // C1: opcode.
    //
    clock();
    verifyRead(0x20FD, 0xD0);

    //
    // C2: offset.
    //
    clock();
    verifyRead(0x20FE, 0x02);

    //
    // NMI edge after C2.
    //
    m_cpu.setNmiLine(true);

    //
    // C3: taken-branch dummy read.
    //
    clock();
    verifyRead(0x20FF, 0xEA);

    //
    // C4: page-crossing dummy read.
    //
    clock();
    verifyRead(0x2001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2101));

    //
    // Branch is finished. NMI starts now.
    // Target opcode must not execute.
    //
    clock();
    verifyRead(0x2101, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2101));
}


void MOS6510TestCpuControl::testNmiPriorityOverIrq()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);
    m_memory.writeRAM(0x2001, 0xEA);

    //
    // NMI vector
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // IRQ vector
    //
    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x50);

    //
    // Start NOP.
    //
    clock();
    verifyRead(0x2000, 0xEA);

    //
    // Both interrupts become pending during the instruction.
    //
    m_cpu.setIrqLine(true);
    m_cpu.setNmiLine(true);

    //
    // Finish NOP.
    //
    clock();
    verifyRead(0x2001, 0xEA);

    //
    // NMI must have priority.
    //
    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));
}
void MOS6510TestCpuControl::testNmiDuringIrqBeforeVector()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x50);

    m_cpu.setIrqLine(true);

    //
    // IRQ C1
    //
    clock();
    verifyRead(0x2000, 0xEA);

    //
    // IRQ C2
    //
    clock();
    verifyRead(0x2000, 0xEA);

    //
    // IRQ C3: PCH
    //
    clock();
    verifyWrite(0x0180, 0x20);

    //
    // IRQ C4: PCL
    //
    clock();
    verifyWrite(0x017F, 0x00);

    //
    // NMI arrives while IRQ sequence is already running.
    //
    m_cpu.setNmiLine(true);

    //
    // IRQ C5 still pushes IRQ-style P.
    //
    clock();
    verifyWrite(0x017E, 0x20);

    //
    // But vector is hijacked by NMI.
    //
    clock();
    verifyRead(0xFFFA, 0x00);

    clock();
    verifyRead(0xFFFB, 0x40);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));

    //
    // Still only one interrupt stack frame.
    //
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
}
void MOS6510TestCpuControl::testNmiDuringIrqTooLateForVector()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x34);
    m_memory.writeRAM(0xFFFB, 0x12);

    m_memory.writeRAM(0xFFFE, 0x78);
    m_memory.writeRAM(0xFFFF, 0x56);

    m_cpu.setIrqLine(true);

    //
    // IRQ C1-C5.
    //
    for (int i = 0; i < 5; ++i)
        clock();

    //
    // IRQ C6: vector low is already IRQ.
    //
    clock();
    verifyRead(0xFFFE, 0x78);

    //
    // NMI arrives too late to change this vector.
    //
    m_cpu.setNmiLine(true);

    //
    // High byte must still come from IRQ vector.
    //
    clock();
    verifyRead(0xFFFF, 0x56);

    QCOMPARE(m_cpu.programCounter(), quint16(0x5678));
}
void MOS6510TestCpuControl::testNmiBeforeBrk()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0x00);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    m_cpu.setNmiLine(true);

    //
    // NMI C1 reads the BRK opcode but does not execute it.
    //
    clock();

    verifyRead(0x2000, 0x00);
    QCOMPARE(m_cpu.programCounter(), quint16(0x2000));

    for (int i = 0; i < 6; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));

    //
    // Only NMI stack frame.
    //
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // Pushed status has B clear.
    //
    QCOMPARE(
        m_memory.readRAM(0x017E) &
            static_cast<quint8>(MOS6510StatusFlag::Break),
        quint8(0x00));
}
void MOS6510TestCpuControl::testNmiDuringBrkBeforeVector()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0x00);
    m_memory.writeRAM(0x2001, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x50);

    //
    // BRK C1.
    //
    clock();
    verifyRead(0x2000, 0x00);

    //
    // BRK C2: padding byte.
    //
    clock();
    verifyRead(0x2001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));

    //
    // BRK C3: PCH.
    //
    clock();
    verifyWrite(0x0180, 0x20);

    //
    // BRK C4: PCL.
    //
    clock();
    verifyWrite(0x017F, 0x02);

    //
    // NMI arrives early enough to hijack BRK.
    //
    m_cpu.setNmiLine(true);

    //
    // C5 remains BRK: B is pushed SET.
    //
    clock();
    verifyWrite(0x017E, 0x30);

    //
    // But vector becomes NMI.
    //
    clock();
    verifyRead(0xFFFA, 0x00);

    clock();
    verifyRead(0xFFFB, 0x40);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));

    //
    // This proves that this was a hijacked BRK,
    // not an ordinary NMI.
    //
    QVERIFY(
        (m_memory.readRAM(0x017E) &
         static_cast<quint8>(MOS6510StatusFlag::Break)) != 0);
}
void MOS6510TestCpuControl::testNmiDuringBrkTooLateForVector()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0x00);
    m_memory.writeRAM(0x2001, 0xEA);

    m_memory.writeRAM(0xFFFA, 0x34);
    m_memory.writeRAM(0xFFFB, 0x12);

    m_memory.writeRAM(0xFFFE, 0x78);
    m_memory.writeRAM(0xFFFF, 0x56);

    //
    // BRK C1-C5.
    //
    for (int i = 0; i < 5; ++i)
        clock();

    //
    // C6 has already selected IRQ/BRK vector.
    //
    clock();
    verifyRead(0xFFFE, 0x78);

    //
    // NMI is now too late to replace only the high byte.
    //
    m_cpu.setNmiLine(true);

    clock();
    verifyRead(0xFFFF, 0x56);

    QCOMPARE(m_cpu.programCounter(), quint16(0x5678));
}


void MOS6510TestCpuControl::testNmiLateDuringIrq()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);

    //
    // NMI vector -> $4000
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // IRQ vector -> $3000
    //
    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x30);

    //
    // First IRQ-handler instruction: NOP
    //
    m_memory.writeRAM(0x3000, 0xEA);
    m_memory.writeRAM(0x3001, 0xEA);

    m_cpu.setIrqLine(true);

    //
    // IRQ C1-C5.
    //
    for (int i = 0; i < 5; ++i)
        clock();

    //
    // IRQ C6: vector low.
    //
    clock();
    verifyRead(0xFFFE, 0x00);

    //
    // NMI becomes active inside the protected vector-fetch
    // window. It is too late to hijack this IRQ.
    //
    m_cpu.setNmiLine(true);

    //
    // IRQ C7: vector high must remain IRQ.
    //
    clock();
    verifyRead(0xFFFF, 0x30);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3000));

    //
    // The first instruction of the IRQ handler really executes.
    //
    // NOP C1
    //
    clock();
    verifyRead(0x3000, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));

    //
    // NOP C2
    //
    clock();
    verifyRead(0x3001, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));

    //
    // Only now may NMI start.
    //
    clock();
    verifyRead(0x3001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));
}
void MOS6510TestCpuControl::testNmiLateDuringBrk()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    //
    // BRK
    //
    m_memory.writeRAM(0x2000, 0x00);
    m_memory.writeRAM(0x2001, 0xEA);

    //
    // NMI -> $4000
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // BRK -> $3000
    //
    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x30);

    //
    // First BRK-handler instruction.
    //
    m_memory.writeRAM(0x3000, 0xEA);
    m_memory.writeRAM(0x3001, 0xEA);

    //
    // BRK C1-C5.
    //
    for (int i = 0; i < 5; ++i)
        clock();

    //
    // BRK C6: vector low.
    //
    clock();
    verifyRead(0xFFFE, 0x00);

    //
    // NMI too late for hijacking.
    //
    m_cpu.setNmiLine(true);

    //
    // BRK C7.
    //
    clock();
    verifyRead(0xFFFF, 0x30);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3000));

    //
    // First handler NOP executes completely.
    //
    clock();
    verifyRead(0x3000, 0xEA);

    clock();
    verifyRead(0x3001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));

    //
    // Now NMI starts.
    //
    clock();
    verifyRead(0x3001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));
}

void MOS6510TestCpuControl::testNmiLostDuringIrqVectorFetch()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x2000, 0xEA);

    //
    // NMI vector -> $4000
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // IRQ vector -> $3000
    //
    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x30);

    //
    // IRQ handler.
    //
    m_memory.writeRAM(0x3000, 0xEA);
    m_memory.writeRAM(0x3001, 0xEA);
    m_memory.writeRAM(0x3002, 0xEA);

    m_cpu.setIrqLine(true);

    //
    // IRQ C1-C5.
    //
    for (int i = 0; i < 5; ++i)
        clock();

    //
    // IRQ C6: vector low has already been selected.
    //
    clock();
    verifyRead(0xFFFE, 0x00);

    //
    // Short NMI pulse inside the protected vector-fetch
    // window.
    //
    m_cpu.setNmiLine(true);
    m_cpu.setNmiLine(false);

    //
    // IRQ C7 must still use the IRQ vector.
    //
    clock();
    verifyRead(0xFFFF, 0x30);

    QCOMPARE(m_cpu.programCounter(), quint16(0x3000));

    //
    // First handler NOP must execute completely.
    //
    clock();
    verifyRead(0x3000, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));

    clock();
    verifyRead(0x3001, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3001));

    //
    // The short NMI pulse must have been lost.
    //
    // Therefore the next handler instruction starts normally.
    //
    clock();
    verifyRead(0x3001, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3002));

    //
    // And its second cycle also executes normally.
    //
    clock();
    verifyRead(0x3002, 0xEA);
    QCOMPARE(m_cpu.programCounter(), quint16(0x3002));

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
}

void MOS6510TestCpuControl::testNmiPriorityOverIrqHandlerStartsNormally()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    //
    // Main program: NOP
    //
    m_memory.writeRAM(0x2000, 0xEA);
    m_memory.writeRAM(0x2001, 0xEA);

    //
    // NMI vector -> $4000
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // IRQ vector -> $5000
    //
    m_memory.writeRAM(0xFFFE, 0x00);
    m_memory.writeRAM(0xFFFF, 0x50);

    //
    // NMI handler: NOP, NOP
    //
    m_memory.writeRAM(0x4000, 0xEA);
    m_memory.writeRAM(0x4001, 0xEA);
    m_memory.writeRAM(0x4002, 0xEA);

    //
    // C1: NOP opcode fetch.
    //
    clock();
    verifyRead(0x2000, 0xEA);

    //
    // Both interrupts become active during the instruction.
    //
    m_cpu.setIrqLine(true);
    m_cpu.setNmiLine(true);

    //
    // C2: finish NOP.
    //
    clock();
    verifyRead(0x2001, 0xEA);

    //
    // NMI has priority and starts now.
    //
    for (int i = 0; i < 7; ++i)
        clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x4000));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // The IRQ that lost against NMI must not start here.
    // The first NMI-handler instruction must execute.
    //
    clock();
    verifyRead(0x4000, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));

    //
    // C2 of handler NOP.
    //
    clock();
    verifyRead(0x4001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x4001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
}
void MOS6510TestCpuControl::testNmiTooLateDuringTwoCycleInstruction()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    //
    // Two NOPs.
    //
    m_memory.writeRAM(0x2000, 0xEA);
    m_memory.writeRAM(0x2001, 0xEA);
    m_memory.writeRAM(0x2002, 0xEA);

    //
    // NMI vector -> $4000
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // C1 of first NOP.
    //
    clock();
    verifyRead(0x2000, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2001));

    //
    // NMI edge after C1.
    //
    // For a two-cycle instruction this is too late for
    // the interrupt poll belonging to this instruction.
    //
    m_cpu.setNmiLine(true);

    //
    // C2 of first NOP.
    //
    clock();
    verifyRead(0x2001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2001));

    //
    // The next instruction must still start normally.
    // NMI must NOT start here.
    //
    clock();
    verifyRead(0x2001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));

    //
    // Finish the second NOP.
    //
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));

    //
    // Now NMI may start.
    //
    clock();
    verifyRead(0x2002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2002));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));
}
void MOS6510TestCpuControl::testNmiTooLateDuringMultiCycleInstruction()
{
    setupCpu();

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setStackPointer(0x80);
    m_cpu.setStatus(0x20);

    //
    // $2000: LDA $1234
    // $2003: NOP
    //
    m_memory.writeRAM(0x2000, 0xAD);
    m_memory.writeRAM(0x2001, 0x34);
    m_memory.writeRAM(0x2002, 0x12);
    m_memory.writeRAM(0x2003, 0xEA);
    m_memory.writeRAM(0x2004, 0xEA);

    m_memory.writeRAM(0x1234, 0x42);

    //
    // NMI vector -> $4000
    //
    m_memory.writeRAM(0xFFFA, 0x00);
    m_memory.writeRAM(0xFFFB, 0x40);

    //
    // LDA C1: opcode.
    //
    clock();
    verifyRead(0x2000, 0xAD);

    //
    // LDA C2: address low.
    //
    clock();
    verifyRead(0x2001, 0x34);

    //
    // LDA C3: address high.
    //
    clock();
    verifyRead(0x2002, 0x12);

    //
    // NMI edge between C3 and C4.
    //
    // The interrupt poll for this instruction has already
    // happened, therefore this NMI is too late to be taken
    // immediately after the LDA.
    //
    m_cpu.setNmiLine(true);

    //
    // LDA C4: operand.
    //
    clock();
    verifyRead(0x1234, 0x42);

    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
    QCOMPARE(m_cpu.programCounter(), quint16(0x2003));

    //
    // The following NOP must still start normally.
    //
    clock();
    verifyRead(0x2003, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2004));

    //
    // Finish NOP.
    //
    clock();
    verifyRead(0x2004, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2004));

    //
    // Only now may NMI start.
    //
    clock();
    verifyRead(0x2004, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x2004));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));
}
