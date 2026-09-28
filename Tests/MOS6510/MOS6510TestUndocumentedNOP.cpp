#include "MOS6510TestUndocumentedNOP.h"

#include <QTest>


MOS6510TestUndocumentedNOP::MOS6510TestUndocumentedNOP()
{
}
MOS6510TestUndocumentedNOP::~MOS6510TestUndocumentedNOP()
{
}


void MOS6510TestUndocumentedNOP::testImplied_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $1A") << quint8(0x1A);
    QTest::newRow("NOP $3A") << quint8(0x3A);
    QTest::newRow("NOP $5A") << quint8(0x5A);
    QTest::newRow("NOP $7A") << quint8(0x7A);
    QTest::newRow("NOP $DA") << quint8(0xDA);
    QTest::newRow("NOP $FA") << quint8(0xFA);
}

void MOS6510TestUndocumentedNOP::testImplied()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xEA);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));

    // Cycle 2: Dummy Read
    clock();
    verifyRead(0x1001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));

    // Cycle 3: Next Opcode Fetch
    // This proves that the undocumented NOP takes exactly two cycles.
    clock();
    verifyRead(0x1001, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}


void MOS6510TestUndocumentedNOP::testImmediate_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $80") << quint8(0x80);
    QTest::newRow("NOP $82") << quint8(0x82);
    QTest::newRow("NOP $89") << quint8(0x89);
    QTest::newRow("NOP $C2") << quint8(0xC2);
    QTest::newRow("NOP $E2") << quint8(0xE2);
}

void MOS6510TestUndocumentedNOP::testImmediate()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x5A);
    m_memory.writeRAM(0x1002, 0xEA);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read ignored immediate operand
    clock();
    verifyRead(0x1001, 0x5A);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));

    // Cycle 3: Next Opcode Fetch
    // This proves that the undocumented NOP takes exactly two cycles.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}


void MOS6510TestUndocumentedNOP::testZeroPage_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $04") << quint8(0x04);
    QTest::newRow("NOP $44") << quint8(0x44);
    QTest::newRow("NOP $64") << quint8(0x64);
}

void MOS6510TestUndocumentedNOP::testZeroPage()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x60);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0060, 0x5A);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read zero-page address
    clock();
    verifyRead(0x1001, 0x60);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Read ignored operand
    clock();
    verifyRead(0x0060, 0x5A);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));
    QCOMPARE(m_memory.readRAM(0x0060), quint8(0x5A));

    // Cycle 4: Next Opcode Fetch
    // This proves that the undocumented NOP takes exactly three cycles.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}


void MOS6510TestUndocumentedNOP::testZeroPageX_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $14") << quint8(0x14);
    QTest::newRow("NOP $34") << quint8(0x34);
    QTest::newRow("NOP $54") << quint8(0x54);
    QTest::newRow("NOP $74") << quint8(0x74);
    QTest::newRow("NOP $D4") << quint8(0xD4);
    QTest::newRow("NOP $F4") << quint8(0xF4);
}

void MOS6510TestUndocumentedNOP::testZeroPageX()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x20);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x60);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0060, 0xA6);
    m_memory.writeRAM(0x0080, 0x5A);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read zero-page base address
    clock();
    verifyRead(0x1001, 0x60);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Dummy Read from unindexed zero-page address
    clock();
    verifyRead(0x0060, 0xA6);

    // Cycle 4: Read ignored operand from indexed zero-page address
    clock();
    verifyRead(0x0080, 0x5A);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x20));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));

    QCOMPARE(m_memory.readRAM(0x0060), quint8(0xA6));
    QCOMPARE(m_memory.readRAM(0x0080), quint8(0x5A));

    // Cycle 5: Next Opcode Fetch
    // This proves that the undocumented NOP takes exactly four cycles.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}


void MOS6510TestUndocumentedNOP::testAbsolute_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $0C") << quint8(0x0C);
}

void MOS6510TestUndocumentedNOP::testAbsolute()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, 0x5A);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read address low
    clock();
    verifyRead(0x1001, 0x34);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Read address high
    clock();
    verifyRead(0x1002, 0x12);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Read ignored operand
    clock();
    verifyRead(0x1234, 0x5A);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));
    QCOMPARE(m_memory.readRAM(0x1234), quint8(0x5A));

    // Cycle 5: Next Opcode Fetch
    // This proves that the undocumented NOP takes exactly four cycles.
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}


void MOS6510TestUndocumentedNOP::testAbsoluteX_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $1C") << quint8(0x1C);
    QTest::newRow("NOP $3C") << quint8(0x3C);
    QTest::newRow("NOP $5C") << quint8(0x5C);
    QTest::newRow("NOP $7C") << quint8(0x7C);
    QTest::newRow("NOP $DC") << quint8(0xDC);
    QTest::newRow("NOP $FC") << quint8(0xFC);
}

void MOS6510TestUndocumentedNOP::testAbsoluteX()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x40);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x4030, 0x5A);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read address low
    clock();
    verifyRead(0x1001, 0x20);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Read address high and add X
    clock();
    verifyRead(0x1002, 0x40);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Read ignored operand
    clock();
    verifyRead(0x4030, 0x5A);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x10));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));
    QCOMPARE(m_memory.readRAM(0x4030), quint8(0x5A));

    // Cycle 5: Next Opcode Fetch
    // No page crossing: exactly four cycles.
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}


void MOS6510TestUndocumentedNOP::testAbsoluteXPageCross_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $1C") << quint8(0x1C);
    QTest::newRow("NOP $3C") << quint8(0x3C);
    QTest::newRow("NOP $5C") << quint8(0x5C);
    QTest::newRow("NOP $7C") << quint8(0x7C);
    QTest::newRow("NOP $DC") << quint8(0xDC);
    QTest::newRow("NOP $FC") << quint8(0xFC);
}

void MOS6510TestUndocumentedNOP::testAbsoluteXPageCross()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x20);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x40);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x4010, 0xA6);
    m_memory.writeRAM(0x4110, 0x5A);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read address low
    clock();
    verifyRead(0x1001, 0xF0);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Read address high and detect page crossing
    clock();
    verifyRead(0x1002, 0x40);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Dummy Read from wrong page
    clock();
    verifyRead(0x4010, 0xA6);

    // Cycle 5: Read ignored operand from corrected address
    clock();
    verifyRead(0x4110, 0x5A);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x20));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));

    QCOMPARE(m_memory.readRAM(0x4010), quint8(0xA6));
    QCOMPARE(m_memory.readRAM(0x4110), quint8(0x5A));

    // Cycle 6: Next Opcode Fetch
    // Page crossing: exactly five cycles.
    clock();
    verifyRead(0x1003, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
}

void MOS6510TestUndocumentedNOP::testZeroPageXWrapAround_data()
{
    QTest::addColumn<quint8>("opcode");

    QTest::newRow("NOP $14") << quint8(0x14);
    QTest::newRow("NOP $34") << quint8(0x34);
    QTest::newRow("NOP $54") << quint8(0x54);
    QTest::newRow("NOP $74") << quint8(0x74);
    QTest::newRow("NOP $D4") << quint8(0xD4);
    QTest::newRow("NOP $F4") << quint8(0xF4);
}

void MOS6510TestUndocumentedNOP::testZeroPageXWrapAround()
{
    QFETCH(quint8, opcode);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x20);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(0xA5);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0xEA);

    //
    // $F0 + X($20) = $10 with zero-page wrap-around.
    //
    m_memory.writeRAM(0x00F0, 0xA6);
    m_memory.writeRAM(0x0010, 0x5A);

    // Cycle 1: Opcode Fetch
    clock();
    verifyRead(0x1000, opcode);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Read zero-page base address
    clock();
    verifyRead(0x1001, 0xF0);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Dummy Read from unindexed zero-page address
    clock();
    verifyRead(0x00F0, 0xA6);

    // Cycle 4: Read ignored operand from wrapped zero-page address
    clock();
    verifyRead(0x0010, 0x5A);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x20));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));
    QCOMPARE(m_cpu.status(), quint8(0xA5));

    QCOMPARE(m_memory.readRAM(0x00F0), quint8(0xA6));
    QCOMPARE(m_memory.readRAM(0x0010), quint8(0x5A));

    // Cycle 5: Next Opcode Fetch
    // This proves that the undocumented NOP still takes
    // exactly four cycles when zero-page indexing wraps.
    clock();
    verifyRead(0x1002, 0xEA);

    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
}
