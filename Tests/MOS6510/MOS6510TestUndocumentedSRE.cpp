#include "MOS6510TestUndocumentedSRE.h"

#include <QTest>


MOS6510TestUndocumentedSRE::MOS6510TestUndocumentedSRE()
{
}
MOS6510TestUndocumentedSRE::~MOS6510TestUndocumentedSRE()
{
}


// -----------------------------------------------------------------------------
// $47 SRE Zero Page
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testZeroPage_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("memory");
    QTest::addColumn<quint8>("status");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedMemory");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");

    QTest::newRow("zero")
        << quint8(0x00)
        << quint8(0x00)
        << quint8(0x20)
        << quint8(0x00)
        << quint8(0x00)
        << false
        << true
        << false;

    QTest::newRow("simple")
        << quint8(0x10)
        << quint8(0x42)
        << quint8(0x20)
        << quint8(0x31)
        << quint8(0x21)
        << false
        << false
        << false;

    QTest::newRow("carry")
        << quint8(0x10)
        << quint8(0x43)
        << quint8(0x20)
        << quint8(0x31)
        << quint8(0x21)
        << true
        << false
        << false;

    QTest::newRow("negative")
        << quint8(0x80)
        << quint8(0x02)
        << quint8(0x20)
        << quint8(0x81)
        << quint8(0x01)
        << false
        << false
        << true;

    QTest::newRow("xor to zero")
        << quint8(0x21)
        << quint8(0x42)
        << quint8(0x20)
        << quint8(0x00)
        << quint8(0x21)
        << false
        << true
        << false;

    //
    // Old Carry must have no influence on LSR.
    //
    QTest::newRow("old carry ignored")
        << quint8(0x10)
        << quint8(0x02)
        << quint8(0x21)
        << quint8(0x11)
        << quint8(0x01)
        << false
        << false
        << false;
}


void MOS6510TestUndocumentedSRE::testZeroPage()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, memory);
    QFETCH(quint8, status);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedMemory);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    setupCpu();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(status);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x47);
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0042, memory);

    // Cycle 1: opcode fetch
    clock();
    verifyRead(0x1000, 0x47);

    // Cycle 2: zero-page address
    clock();
    verifyRead(0x1001, 0x42);

    // Cycle 3: read old memory
    clock();
    verifyRead(0x0042, memory);

    // Cycle 4: RMW dummy write of old value
    clock();
    verifyWrite(0x0042, memory);

    // Cycle 5: write shifted value + EOR
    clock();
    verifyWrite(0x0042, expectedMemory);

    QCOMPARE(m_memory.readRAM(0x0042), expectedMemory);

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), quint8(0x37));
    QCOMPARE(m_cpu.yRegister(), quint8(0x59));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xA5));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        expectedCarry);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Zero),
        expectedZero);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Negative),
        expectedNegative);

    //
    // V, D, I, B and U must be preserved.
    //
    const quint8 preservedMask =
        static_cast<quint8>(
            static_cast<quint8>(MOS6510StatusFlag::Overflow)
            | static_cast<quint8>(MOS6510StatusFlag::Decimal)
            | static_cast<quint8>(MOS6510StatusFlag::InterruptDisable)
            | static_cast<quint8>(MOS6510StatusFlag::Break)
            | static_cast<quint8>(MOS6510StatusFlag::Unused));

    QCOMPARE(
        m_cpu.status() & preservedMask,
        status & preservedMask);

    // Cycle 6: next fetch proves exact 5-cycle timing
    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// $57 SRE Zero Page,X
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testZeroPageX()
{
    setupCpu();

    m_cpu.setAccumulator(0x10);
    m_cpu.setXRegister(0x10);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x7C);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x57);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x66);
    m_memory.writeRAM(0x0030, 0x43);

    clock();
    verifyRead(0x1000, 0x57);

    clock();
    verifyRead(0x1001, 0x20);

    // Indexed zero-page dummy read
    clock();
    verifyRead(0x0020, 0x66);

    clock();
    verifyRead(0x0030, 0x43);

    clock();
    verifyWrite(0x0030, 0x43);

    clock();
    verifyWrite(0x0030, 0x21);

    QCOMPARE(m_memory.readRAM(0x0030), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x31));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Zero),
        false);

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Negative),
        false);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedSRE::testZeroPageXWrap()
{
    setupCpu();

    m_cpu.setAccumulator(0x80);
    m_cpu.setXRegister(0x20);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x21);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x57);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x00F0, 0x66);
    m_memory.writeRAM(0x0010, 0x03);

    clock();
    verifyRead(0x1000, 0x57);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x00F0, 0x66);

    clock();
    verifyRead(0x0010, 0x03);

    clock();
    verifyWrite(0x0010, 0x03);

    clock();
    verifyWrite(0x0010, 0x01);

    QCOMPARE(m_memory.readRAM(0x0010), quint8(0x01));
    QCOMPARE(m_cpu.accumulator(), quint8(0x81));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// $4F SRE Absolute
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testAbsolute()
{
    setupCpu();

    m_cpu.setAccumulator(0x14);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x4F);
    m_memory.writeRAM(0x1001, 0x34);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1234, 0x42);

    clock();
    verifyRead(0x1000, 0x4F);

    clock();
    verifyRead(0x1001, 0x34);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1234, 0x42);

    clock();
    verifyWrite(0x1234, 0x42);

    clock();
    verifyWrite(0x1234, 0x21);

    QCOMPARE(m_memory.readRAM(0x1234), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x35));

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// $5F SRE Absolute,X
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testAbsoluteX()
{
    setupCpu();

    m_cpu.setAccumulator(0x10);
    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x5F);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1225, 0x43);

    clock();
    verifyRead(0x1000, 0x5F);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    //
    // Fixed RMW dummy cycle even without page crossing.
    //
    clock();
    verifyRead(0x1225, 0x43);

    clock();
    verifyRead(0x1225, 0x43);

    clock();
    verifyWrite(0x1225, 0x43);

    clock();
    verifyWrite(0x1225, 0x21);

    QCOMPARE(m_memory.readRAM(0x1225), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x31));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedSRE::testAbsoluteXPageCross()
{
    setupCpu();

    m_cpu.setAccumulator(0x80);
    m_cpu.setXRegister(0x20);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x5F);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    // $12F0 + X $20 = $1310
    // Wrong-page dummy address = $1210
    m_memory.writeRAM(0x1210, 0x66);
    m_memory.writeRAM(0x1310, 0x03);

    clock();
    verifyRead(0x1000, 0x5F);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1210, 0x66);

    clock();
    verifyRead(0x1310, 0x03);

    clock();
    verifyWrite(0x1310, 0x03);

    clock();
    verifyWrite(0x1310, 0x01);

    QCOMPARE(m_memory.readRAM(0x1310), quint8(0x01));
    QCOMPARE(m_cpu.accumulator(), quint8(0x81));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// $5B SRE Absolute,Y
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testAbsoluteY()
{
    setupCpu();

    m_cpu.setAccumulator(0x08);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x05);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x5B);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1225, 0x42);

    clock();
    verifyRead(0x1000, 0x5B);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1225, 0x42);

    clock();
    verifyRead(0x1225, 0x42);

    clock();
    verifyWrite(0x1225, 0x42);

    clock();
    verifyWrite(0x1225, 0x21);

    QCOMPARE(m_memory.readRAM(0x1225), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x29));

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestUndocumentedSRE::testAbsoluteYPageCross()
{
    setupCpu();

    m_cpu.setAccumulator(0x04);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x20);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x5B);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0x12);
    m_memory.writeRAM(0x1003, 0xEA);

    m_memory.writeRAM(0x1210, 0x66);
    m_memory.writeRAM(0x1310, 0x83);

    clock();
    verifyRead(0x1000, 0x5B);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x1002, 0x12);

    clock();
    verifyRead(0x1210, 0x66);

    clock();
    verifyRead(0x1310, 0x83);

    clock();
    verifyWrite(0x1310, 0x83);

    clock();
    verifyWrite(0x1310, 0x41);

    QCOMPARE(m_memory.readRAM(0x1310), quint8(0x41));
    QCOMPARE(m_cpu.accumulator(), quint8(0x45));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// $43 SRE (Indirect,X)
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testIndexedIndirect()
{
    setupCpu();

    m_cpu.setAccumulator(0x10);
    m_cpu.setXRegister(0x04);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x43);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0020, 0x66);

    m_memory.writeRAM(0x0024, 0x34);
    m_memory.writeRAM(0x0025, 0x12);

    m_memory.writeRAM(0x1234, 0x43);

    clock();
    verifyRead(0x1000, 0x43);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x66);

    clock();
    verifyRead(0x0024, 0x34);

    clock();
    verifyRead(0x0025, 0x12);

    clock();
    verifyRead(0x1234, 0x43);

    clock();
    verifyWrite(0x1234, 0x43);

    clock();
    verifyWrite(0x1234, 0x21);

    QCOMPARE(m_memory.readRAM(0x1234), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x31));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedSRE::testIndexedIndirectPointerWrap()
{
    setupCpu();

    m_cpu.setAccumulator(0x80);
    m_cpu.setXRegister(0x0F);
    m_cpu.setYRegister(0x59);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x43);
    m_memory.writeRAM(0x1001, 0xF0);
    m_memory.writeRAM(0x1002, 0xEA);

    // $F0 + X($0F) = $FF
    m_memory.writeRAM(0x00F0, 0x66);

    // Pointer high wraps from $FF to $00
    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x0000, 0x12);

    m_memory.writeRAM(0x1234, 0x03);

    clock();
    verifyRead(0x1000, 0x43);

    clock();
    verifyRead(0x1001, 0xF0);

    clock();
    verifyRead(0x00F0, 0x66);

    clock();
    verifyRead(0x00FF, 0x34);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1234, 0x03);

    clock();
    verifyWrite(0x1234, 0x03);

    clock();
    verifyWrite(0x1234, 0x01);

    QCOMPARE(m_memory.readRAM(0x1234), quint8(0x01));
    QCOMPARE(m_cpu.accumulator(), quint8(0x81));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// $53 SRE (Indirect),Y
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testIndirectIndexed()
{
    setupCpu();

    m_cpu.setAccumulator(0x10);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x05);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x53);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0040, 0x20);
    m_memory.writeRAM(0x0041, 0x12);

    m_memory.writeRAM(0x1225, 0x43);

    clock();
    verifyRead(0x1000, 0x53);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x0040, 0x20);

    clock();
    verifyRead(0x0041, 0x12);

    // Fixed indexed RMW dummy cycle
    clock();
    verifyRead(0x1225, 0x43);

    clock();
    verifyRead(0x1225, 0x43);

    clock();
    verifyWrite(0x1225, 0x43);

    clock();
    verifyWrite(0x1225, 0x21);

    QCOMPARE(m_memory.readRAM(0x1225), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x31));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedSRE::testIndirectIndexedPageCross()
{
    setupCpu();

    m_cpu.setAccumulator(0x04);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x20);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x53);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0xEA);

    m_memory.writeRAM(0x0040, 0xF0);
    m_memory.writeRAM(0x0041, 0x12);

    // $12F0 + Y $20 = $1310
    // Wrong-page dummy address = $1210
    m_memory.writeRAM(0x1210, 0x66);
    m_memory.writeRAM(0x1310, 0x83);

    clock();
    verifyRead(0x1000, 0x53);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x0040, 0xF0);

    clock();
    verifyRead(0x0041, 0x12);

    clock();
    verifyRead(0x1210, 0x66);

    clock();
    verifyRead(0x1310, 0x83);

    clock();
    verifyWrite(0x1310, 0x83);

    clock();
    verifyWrite(0x1310, 0x41);

    QCOMPARE(m_memory.readRAM(0x1310), quint8(0x41));
    QCOMPARE(m_cpu.accumulator(), quint8(0x45));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestUndocumentedSRE::testIndirectIndexedPointerWrap()
{
    setupCpu();

    m_cpu.setAccumulator(0x10);
    m_cpu.setXRegister(0x37);
    m_cpu.setYRegister(0x05);
    m_cpu.setStackPointer(0xA5);
    m_cpu.setStatus(0x20);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x53);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    // Pointer low at $FF, high wraps to $00
    m_memory.writeRAM(0x00FF, 0x20);
    m_memory.writeRAM(0x0000, 0x12);

    m_memory.writeRAM(0x1225, 0x43);

    clock();
    verifyRead(0x1000, 0x53);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x20);

    clock();
    verifyRead(0x0000, 0x12);

    clock();
    verifyRead(0x1225, 0x43);

    clock();
    verifyRead(0x1225, 0x43);

    clock();
    verifyWrite(0x1225, 0x43);

    clock();
    verifyWrite(0x1225, 0x21);

    QCOMPARE(m_memory.readRAM(0x1225), quint8(0x21));
    QCOMPARE(m_cpu.accumulator(), quint8(0x31));

    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Carry),
        true);

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// Exhaustive semantics
//
// Test all 256 accumulator values against all 256 memory values and both
// possible old Carry states.
//
// SRE:
//     C    = bit 0 of old memory
//     Mnew = Mold >> 1
//     Anew = Aold XOR Mnew
//     Z    = Anew == 0
//     N    = bit 7 of Anew
//
// Old Carry must have no influence on the LSR operation.
// V, D, I, B and U must remain unchanged.
// -----------------------------------------------------------------------------

void MOS6510TestUndocumentedSRE::testExhaustive()
{
    setupCpu();

    m_memory.writeRAM(0x1000, 0x47);
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    const quint8 preservedMask =
        static_cast<quint8>(
            static_cast<quint8>(MOS6510StatusFlag::Overflow)
            | static_cast<quint8>(MOS6510StatusFlag::Decimal)
            | static_cast<quint8>(MOS6510StatusFlag::InterruptDisable)
            | static_cast<quint8>(MOS6510StatusFlag::Break)
            | static_cast<quint8>(MOS6510StatusFlag::Unused));

    for (quint16 accumulator = 0;
         accumulator <= 0xFF;
         ++accumulator)
    {
        for (quint16 memory = 0;
             memory <= 0xFF;
             ++memory)
        {
            for (quint8 oldCarry = 0;
                 oldCarry <= 1;
                 ++oldCarry)
            {
                const quint8 a =
                    static_cast<quint8>(accumulator);

                const quint8 oldMemory =
                    static_cast<quint8>(memory);

                const quint8 newMemory =
                    static_cast<quint8>(oldMemory >> 1);

                const quint8 expectedAccumulator =
                    static_cast<quint8>(a ^ newMemory);

                const bool expectedCarry =
                    (oldMemory & 0x01) != 0;

                const bool expectedZero =
                    expectedAccumulator == 0;

                const bool expectedNegative =
                    (expectedAccumulator & 0x80) != 0;

                //
                // V, D, I, B and U deliberately set.
                // C is varied independently.
                //
                quint8 initialStatus = 0x7C;

                if (oldCarry != 0)
                {
                    initialStatus |=
                        static_cast<quint8>(
                            MOS6510StatusFlag::Carry);
                }
                else
                {
                    initialStatus &=
                        static_cast<quint8>(
                            ~static_cast<quint8>(
                                MOS6510StatusFlag::Carry));
                }

                m_cpu.initialize();

                m_cpu.setAccumulator(a);
                m_cpu.setXRegister(0x37);
                m_cpu.setYRegister(0x59);
                m_cpu.setStackPointer(0xA5);
                m_cpu.setStatus(initialStatus);
                m_cpu.setProgramCounter(0x1000);

                m_memory.writeRAM(0x0042, oldMemory);

                //
                // Exact SRE zp timing: 5 cycles.
                //
                clock();
                verifyRead(0x1000, 0x47);

                clock();
                verifyRead(0x1001, 0x42);

                clock();
                verifyRead(0x0042, oldMemory);

                clock();
                verifyWrite(0x0042, oldMemory);

                clock();
                verifyWrite(0x0042, newMemory);

                const quint8 actualStatus =
                    m_cpu.status();

                if (m_memory.readRAM(0x0042) != newMemory
                    || m_cpu.accumulator() != expectedAccumulator
                    || m_cpu.xRegister() != 0x37
                    || m_cpu.yRegister() != 0x59
                    || m_cpu.stackPointer() != 0xA5
                    || m_cpu.statusFlag(MOS6510StatusFlag::Carry)
                           != expectedCarry
                    || m_cpu.statusFlag(MOS6510StatusFlag::Zero)
                           != expectedZero
                    || m_cpu.statusFlag(MOS6510StatusFlag::Negative)
                           != expectedNegative
                    || (actualStatus & preservedMask)
                           != (initialStatus & preservedMask))
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "SRE mismatch: "
                                "A=$%1 M=$%2 C=%3 | "
                                "actual A=$%4 M=$%5 P=$%6 | "
                                "expected A=$%7 M=$%8 "
                                "C=%9 Z=%10 N=%11")
                                .arg(a, 2, 16, QLatin1Char('0'))
                                .arg(
                                    oldMemory,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(oldCarry)
                                .arg(
                                    m_cpu.accumulator(),
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    m_memory.readRAM(0x0042),
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    actualStatus,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    expectedAccumulator,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(
                                    newMemory,
                                    2,
                                    16,
                                    QLatin1Char('0'))
                                .arg(expectedCarry ? 1 : 0)
                                .arg(expectedZero ? 1 : 0)
                                .arg(expectedNegative ? 1 : 0)));
                }

                //
                // Next fetch proves SRE zp consumed exactly 5 cycles.
                //
                clock();
                verifyRead(0x1002, 0xEA);
            }
        }
    }
}
