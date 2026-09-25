#include "MOS6510TestControlFlow.h"

#include <QTest>


MOS6510TestControlFlow::MOS6510TestControlFlow()
{
}

MOS6510TestControlFlow::~MOS6510TestControlFlow()
{
}


// -----------------------------------------------------------------------------
// JMP
// -----------------------------------------------------------------------------

void MOS6510TestControlFlow::testJmpAbsolute()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x4C); // JMP $3456
    m_memory.writeRAM(0x1001, 0x56);
    m_memory.writeRAM(0x1002, 0x34);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    // Next opcode fetch.
    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testJmpAbsoluteProgramCounterWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0xFFFE);

    m_memory.writeRAM(0xFFFE, 0x4C);
    m_memory.writeRAM(0xFFFF, 0x56);
    m_memory.writeRAM(0x0000, 0x34);
    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0xFFFF));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x0000));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testJmpIndirect()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0x6C); // JMP ($2000)
    m_memory.writeRAM(0x1001, 0x00);
    m_memory.writeRAM(0x1002, 0x20);

    m_memory.writeRAM(0x2000, 0x56);
    m_memory.writeRAM(0x2001, 0x34);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testJmpIndirectPageBoundaryBug()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    // NMOS 6502/6510 JMP ($20FF) bug:
    //
    // low byte  = $20FF
    // high byte = $2000
    //
    // NOT $2100.
    m_memory.writeRAM(0x1000, 0x6C);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0x20);

    m_memory.writeRAM(0x20FF, 0x56);
    m_memory.writeRAM(0x2000, 0x34);

    // Deliberately different value at the address a "fixed"
    // implementation would read.
    m_memory.writeRAM(0x2100, 0xAB);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testJmpIndirectPointerWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    // JMP ($FFFF) on NMOS 6502/6510:
    // low byte  from $FFFF
    // high byte from $FF00 because of the page-boundary bug.
    m_memory.writeRAM(0x1000, 0x6C);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xFF);

    m_memory.writeRAM(0xFFFF, 0x56);
    m_memory.writeRAM(0xFF00, 0x34);

    // Must not be used as high byte.
    m_memory.writeRAM(0x0000, 0xAB);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}


// -----------------------------------------------------------------------------
// JSR
// -----------------------------------------------------------------------------

void MOS6510TestControlFlow::testJsr()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFF);

    m_memory.writeRAM(0x1000, 0x20); // JSR $3456
    m_memory.writeRAM(0x1001, 0x56);
    m_memory.writeRAM(0x1002, 0x34);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    m_cpu.clock();

    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0x10));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));

    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(0x01FE), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testJsrStackPointerWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0x00);

    m_memory.writeRAM(0x1000, 0x20);
    m_memory.writeRAM(0x1001, 0x56);
    m_memory.writeRAM(0x1002, 0x34);
    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0100), quint8(0x10));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));

    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
}

void MOS6510TestControlFlow::testJsrProgramCounterWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0xFFFE);
    m_cpu.setStackPointer(0xFF);

    m_memory.writeRAM(0xFFFE, 0x20);
    m_memory.writeRAM(0xFFFF, 0x56);
    m_memory.writeRAM(0x0000, 0x34);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    // JSR pushes the address of the final operand byte.
    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0x00));
    QCOMPARE(m_memory.readRAM(0x01FE), quint8(0x00));

    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
}


// -----------------------------------------------------------------------------
// RTS
// -----------------------------------------------------------------------------

void MOS6510TestControlFlow::testRts()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFD);

    m_memory.writeRAM(0x1000, 0x60); // RTS

    // Stored JSR return address = $3456.
    // RTS returns to $3457.
    m_memory.writeRAM(0x01FE, 0x56);
    m_memory.writeRAM(0x01FF, 0x34);

    m_memory.writeRAM(0x3457, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3458));
}

void MOS6510TestControlFlow::testRtsStackPointerWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFF);

    m_memory.writeRAM(0x1000, 0x60);

    // First pull wraps SP $FF -> $00.
    m_memory.writeRAM(0x0100, 0x56);
    m_memory.writeRAM(0x0101, 0x34);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.stackPointer(), quint8(0x01));
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testRtsProgramCounterWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFD);

    m_memory.writeRAM(0x1000, 0x60);

    // Pulled return address $FFFF.
    // RTS adds one -> $0000.
    m_memory.writeRAM(0x01FE, 0xFF);
    m_memory.writeRAM(0x01FF, 0xFF);

    m_memory.writeRAM(0x0000, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x0000));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x0001));
}


// -----------------------------------------------------------------------------
// BRK
// -----------------------------------------------------------------------------

void MOS6510TestControlFlow::testBrk()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x29);

    m_memory.writeRAM(0x1000, 0x00); // BRK
    m_memory.writeRAM(0x1001, 0xEA); // padding byte

    m_memory.writeRAM(0xFFFE, 0x56);
    m_memory.writeRAM(0xFFFF, 0x34);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    // BRK pushes PC + 2.
    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0x10));
    QCOMPARE(m_memory.readRAM(0x01FE), quint8(0x02));

    // Pushed status has B and unused bits set.
    QCOMPARE(m_memory.readRAM(0x01FD), quint8(0x39));

    QCOMPARE(m_cpu.stackPointer(), quint8(0xFC));
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    // I is set by BRK. Decimal remains unchanged on NMOS 6502/6510.
    QCOMPARE(m_cpu.status(), quint8(0x2D));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testBrkStackPointerWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0x01);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0x1000, 0x00);
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0xFFFE, 0x56);
    m_memory.writeRAM(0xFFFF, 0x34);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_memory.readRAM(0x0101), quint8(0x10));
    QCOMPARE(m_memory.readRAM(0x0100), quint8(0x02));

    // Third push wraps to $01FF.
    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0x30));

    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
}

void MOS6510TestControlFlow::testBrkProgramCounterWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0xFFFF);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x20);

    m_memory.writeRAM(0xFFFF, 0x00);

    m_memory.writeRAM(0xFFFE, 0x56);

    // Notice that $FFFF is both the BRK opcode location and
    // the high byte of the IRQ/BRK vector. The vector read
    // therefore sees the byte stored at $FFFF.
    //
    // Use a vector low byte that makes the resulting target
    // unambiguous.
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    // Opcode at $FFFF advances to $0000 and BRK's padding
    // cycle advances once more to $0001.
    QCOMPARE(m_memory.readRAM(0x01FF), quint8(0x00));
    QCOMPARE(m_memory.readRAM(0x01FE), quint8(0x01));
}


// -----------------------------------------------------------------------------
// RTI
// -----------------------------------------------------------------------------

void MOS6510TestControlFlow::testRti()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFC);
    m_cpu.setStatus(0x00);

    m_memory.writeRAM(0x1000, 0x40); // RTI

    m_memory.writeRAM(0x01FD, 0xED);
    m_memory.writeRAM(0x01FE, 0x56);
    m_memory.writeRAM(0x01FF, 0x34);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));

    // RTI restores the status byte from the stack.
    // Bit 5 (Unused) is always set in the emulated status register.
    QCOMPARE(m_cpu.status(), quint8(0xED));

    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
}

void MOS6510TestControlFlow::testRtiStackPointerWrapAround()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFE);
    m_cpu.setStatus(0x00);

    m_memory.writeRAM(0x1000, 0x40);

    m_memory.writeRAM(0x01FF, 0x24);
    m_memory.writeRAM(0x0100, 0x56);
    m_memory.writeRAM(0x0101, 0x34);

    m_memory.writeRAM(0x3456, 0xEA);

    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();
    m_cpu.clock();

    QCOMPARE(m_cpu.stackPointer(), quint8(0x01));
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.status(), quint8(0x24));
}

void MOS6510TestControlFlow::testRtiStatus_data()
{
    QTest::addColumn<quint8>("stackStatus");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("all clear")
        << quint8(0x00)
        << quint8(0x20);

    QTest::newRow("break clear")
        << quint8(0x45)
        << quint8(0x65);

    QTest::newRow("break set")
        << quint8(0x55)
        << quint8(0x75);

    QTest::newRow("unused clear")
        << quint8(0x8D)
        << quint8(0xAD);

    QTest::newRow("all set")
        << quint8(0xFF)
        << quint8(0xFF);
}

void MOS6510TestControlFlow::testRtiStatus()
{
    QFETCH(quint8, stackStatus);
    QFETCH(quint8, expectedStatus);

    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFC);
    m_cpu.setStatus(0x00);

    m_memory.writeRAM(0x1000, 0x40); // RTI
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FD, stackStatus);
    m_memory.writeRAM(0x01FE, 0x56);
    m_memory.writeRAM(0x01FF, 0x34);

    m_memory.writeRAM(0x3456, 0xEA);

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFC));
    QCOMPARE(m_cpu.status(), quint8(0x00));

    // Cycle 2: dummy read from program counter
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFC));
    QCOMPARE(m_cpu.status(), quint8(0x00));

    // Cycle 3: dummy read from current stack position
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFC));
    QCOMPARE(m_cpu.status(), quint8(0x00));

    // Cycle 4: pull status
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFD));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: pull PCL
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 6: pull PCH and restore PC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3456));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 7: fetch first opcode at restored PC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x3457));
    QCOMPARE(m_cpu.status(), expectedStatus);
}
