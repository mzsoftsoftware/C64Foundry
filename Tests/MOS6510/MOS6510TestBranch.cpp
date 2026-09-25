#include "MOS6510TestBranch.h"

#include <QTest>


MOS6510TestBranch::MOS6510TestBranch()
{
}

MOS6510TestBranch::~MOS6510TestBranch()
{
}


// --------------------------------------------------------------------------------------------
void MOS6510TestBranch::testBranchNotTaken_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("status");

    QTest::newRow("BCC") << quint8(0x90) << quint8(0x01);
    QTest::newRow("BCS") << quint8(0xB0) << quint8(0x00);

    QTest::newRow("BEQ") << quint8(0xF0) << quint8(0x00);
    QTest::newRow("BNE") << quint8(0xD0) << quint8(0x02);

    QTest::newRow("BMI") << quint8(0x30) << quint8(0x00);
    QTest::newRow("BPL") << quint8(0x10) << quint8(0x80);

    QTest::newRow("BVC") << quint8(0x50) << quint8(0x40);
    QTest::newRow("BVS") << quint8(0x70) << quint8(0x00);
}

void MOS6510TestBranch::testBranchNotTaken()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, status);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read relative offset.
    // Branch is not taken, therefore the instruction ends here.
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), status);

    // Cycle 3: Next Opcode Fetch
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}


// --------------------------------------------------------------------------------------------
void MOS6510TestBranch::testBranchTakenForward_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("status");

    QTest::newRow("BCC") << quint8(0x90) << quint8(0x00);
    QTest::newRow("BCS") << quint8(0xB0) << quint8(0x01);

    QTest::newRow("BEQ") << quint8(0xF0) << quint8(0x02);
    QTest::newRow("BNE") << quint8(0xD0) << quint8(0x00);

    QTest::newRow("BMI") << quint8(0x30) << quint8(0x80);
    QTest::newRow("BPL") << quint8(0x10) << quint8(0x00);

    QTest::newRow("BVC") << quint8(0x50) << quint8(0x00);
    QTest::newRow("BVS") << quint8(0x70) << quint8(0x40);
}

void MOS6510TestBranch::testBranchTakenForward()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, status);

    setupCpu();

    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x10);

    // Base after reading operand = $1002
    // $1002 + $10 = $1012
    m_memory.writeRAM(0x1012, 0xEA);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Extra cycle for taken branch.
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1012));

    // Next opcode fetch.
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1013));

    QCOMPARE(m_cpu.status(), status);
}


// --------------------------------------------------------------------------------------------
void MOS6510TestBranch::testBranchTakenBackward_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("status");

    QTest::newRow("BCC") << quint8(0x90) << quint8(0x00);
    QTest::newRow("BCS") << quint8(0xB0) << quint8(0x01);

    QTest::newRow("BEQ") << quint8(0xF0) << quint8(0x02);
    QTest::newRow("BNE") << quint8(0xD0) << quint8(0x00);

    QTest::newRow("BMI") << quint8(0x30) << quint8(0x80);
    QTest::newRow("BPL") << quint8(0x10) << quint8(0x00);

    QTest::newRow("BVC") << quint8(0x50) << quint8(0x00);
    QTest::newRow("BVS") << quint8(0x70) << quint8(0x40);
}

void MOS6510TestBranch::testBranchTakenBackward()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, status);

    setupCpu();

    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1010);

    m_memory.writeRAM(0x1010, opcode);
    m_memory.writeRAM(0x1011, 0xF8);

    // Base after operand = $1012
    // $F8 = -8
    // $1012 - 8 = $100A
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1011));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1012));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x100A));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x100B));

    QCOMPARE(m_cpu.status(), status);
}


// --------------------------------------------------------------------------------------------
void MOS6510TestBranch::testBranchTakenForwardPageCrossing_data()
{
    testBranchTakenForward_data();
}

void MOS6510TestBranch::testBranchTakenForwardPageCrossing()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, status);

    setupCpu();

    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x10F0);

    m_memory.writeRAM(0x10F0, opcode);
    m_memory.writeRAM(0x10F1, 0x20);

    // Base = $10F2
    // $10F2 + $20 = $1112
    m_memory.writeRAM(0x1112, 0xEA);

    // Cycle 1: Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x10F1));

    // Cycle 2: Offset
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x10F2));

    // Cycle 3: Taken branch
    m_cpu.clock();

    // The architectural PC may already contain the final target.
    QCOMPARE(m_cpu.programCounter(), quint16(0x1112));

    // Cycle 4: Page-crossing penalty
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1112));

    // Cycle 5: Target opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1113));

    QCOMPARE(m_cpu.status(), status);
}


// --------------------------------------------------------------------------------------------
void MOS6510TestBranch::testBranchTakenBackwardPageCrossing_data()
{
    testBranchTakenForward_data();
}

void MOS6510TestBranch::testBranchTakenBackwardPageCrossing()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, status);

    setupCpu();

    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1100);

    m_memory.writeRAM(0x1100, opcode);
    m_memory.writeRAM(0x1101, 0xFC);

    // Base = $1102
    // $FC = -4
    // $1102 - 4 = $10FE
    m_memory.writeRAM(0x10FE, 0xEA);

    // Cycle 1: Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1101));

    // Cycle 2: Offset
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1102));

    // Cycle 3: Taken branch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x10FE));

    // Cycle 4: Page-crossing penalty
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x10FE));

    // Cycle 5: Target opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x10FF));

    QCOMPARE(m_cpu.status(), status);
}
