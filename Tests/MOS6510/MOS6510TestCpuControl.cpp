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

    // C1
    clock();
    verifyRead(0x3456, 0xEA);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C2
    clock();
    verifyRead(0x3456, 0xEA);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x80));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C3
    clock();
    verifyWrite(0x0180, 0x34);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7F));

    // C4
    clock();
    verifyWrite(0x017F, 0x56);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7E));
    QVERIFY(!m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C5
    clock();
    verifyWrite(0x017E, 0x29);

    QCOMPARE(m_cpu.stackPointer(), quint8(0x7D));
    QVERIFY(m_cpu.statusFlag(MOS6510StatusFlag::InterruptDisable));

    // C6
    clock();
    verifyRead(0xFFFE, 0xCD);

    // C7
    clock();
    verifyRead(0xFFFF, 0xAB);

    QCOMPARE(m_cpu.programCounter(), quint16(0xABCD));

    // C8 - first opcode fetch from IRQ handler
    clock();
    verifyRead(0xABCD, 0xEA);
}
