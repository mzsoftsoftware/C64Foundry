#include "MOS6510TestStack.h"

#include <QTest>

MOS6510TestStack::MOS6510TestStack()
{
}

MOS6510TestStack::~MOS6510TestStack()
{
}

void MOS6510TestStack::initializeRegisters()
{
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x7D);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestStack::testPush_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("PHA zero") << quint8(0x48) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("PHA positive") << quint8(0x48) << quint8(0x42) << quint8(0x7D);
    QTest::newRow("PHA negative") << quint8(0x48) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("PHP") << quint8(0x08) << quint8(0x00) << quint8(0x4D);
}
void MOS6510TestStack::testPush()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStatus(status);
    if (opcode == 0x48)
        m_cpu.setAccumulator(value);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xEA);

    const quint8 expectedAccumulator = m_cpu.accumulator();
    const quint8 expectedXRegister = m_cpu.xRegister();
    const quint8 expectedYRegister = m_cpu.yRegister();
    const quint8 expectedStackPointer = 0xFE;
    quint8 expectedStackValue = value;
    if (opcode == 0x08)
        expectedStackValue = status | 0x30;

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 2: push
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), expectedStackPointer);
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), expectedXRegister);
    QCOMPARE(m_cpu.yRegister(), expectedYRegister);
    QCOMPARE(m_cpu.status(), status);
    QCOMPARE(m_memory.readRAM(0x01FF), expectedStackValue);

    // Cycle 3: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStack::testPushWrap_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("PHA") << quint8(0x48) << quint8(0x42) << quint8(0x7D);
    QTest::newRow("PHP") << quint8(0x08) << quint8(0x00) << quint8(0x4D);
}
void MOS6510TestStack::testPushWrap()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0x00);
    m_cpu.setAccumulator(value);
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xEA);

    quint8 expectedStackValue = value;
    if (opcode == 0x08)
        expectedStackValue = status | 0x30;

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x00));

    // Cycle 2: push
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_memory.readRAM(0x0100), expectedStackValue);

    // Cycle 3: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStack::testPullAccumulator_data()
{
    QTest::addColumn<quint8>("value");

    QTest::newRow("zero") << quint8(0x00);
    QTest::newRow("positive") << quint8(0x42);
    QTest::newRow("negative") << quint8(0x80);
}

void MOS6510TestStack::testPullAccumulator()
{
    QFETCH(quint8, value);

    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFE);
    m_cpu.setAccumulator(0x11);
    m_cpu.setStatus(0x7D);
    m_memory.writeRAM(0x1000, 0x68); // PLA
    m_memory.writeRAM(0x1001, 0xEA); // NOP

    // Stack contains the value to pull.
    m_memory.writeRAM(0x01FF, value);

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: stack dummy cycle
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 3: pull
    m_cpu.clock();
    quint8 expectedStatus = 0x7D;
    if (value == 0x00)
        expectedStatus |= 0x02;
    else
        expectedStatus &= ~quint8(0x02);
    if (value & 0x80)
        expectedStatus |= 0x80;
    else
        expectedStatus &= ~quint8(0x80);
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.accumulator(), value);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStack::testPullAccumulatorWrap()
{
    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setAccumulator(0x11);
    m_cpu.setStatus(0x7D);
    m_memory.writeRAM(0x1000, 0x68); // PLA
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    // SP wraps from $FF to $00.
    // Therefore PLA must read from $0100.
    m_memory.writeRAM(0x0100, 0x80);

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 2: stack dummy cycle
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));

    // Cycle 3: pull
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x00));
    QCOMPARE(m_cpu.accumulator(), quint8(0x80));
    // $80 sets Negative and clears Zero.
    QCOMPARE(m_cpu.status(), quint8(0xFD));

    // Cycle 4: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStack::testPullStatus_data()
{
    QTest::addColumn<quint8>("value");

    QTest::newRow("all clear") << quint8(0x00);
    QTest::newRow("carry and decimal") << quint8(0x09);
    QTest::newRow("negative and overflow") << quint8(0xC0);
    QTest::newRow("all flags") << quint8(0xFF);
}
void MOS6510TestStack::testPullStatus()
{
    QFETCH(quint8, value);

    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFE);
    m_cpu.setAccumulator(0x11);
    m_cpu.setStatus(0x7D);
    m_memory.writeRAM(0x1000, 0x28); // PLP
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    // Stack contains the status value to pull.
    m_memory.writeRAM(0x01FF, value);

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: stack dummy cycle
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFE));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 3: pull status
    m_cpu.clock();
    // Bit 5 (Unused) is always set in the CPU status register.
    const quint8 expectedStatus = value | 0x20;
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStack::testPullStatusWrap()
{
    setupCpu();
    initializeRegisters();
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x7D);
    m_memory.writeRAM(0x1000, 0x28); // PLP
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    // SP wraps from $FF to $00.
    // Therefore PLP must read from $0100.
    m_memory.writeRAM(0x0100, 0xC0);

    // Cycle 1: opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 2: stack dummy cycle
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 3: pull status
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x00));
    // $C0 = Negative + Overflow.
    // Bit 5 (Unused) is always set in the CPU status register.
    QCOMPARE(m_cpu.status(), quint8(0xE0));

    // Cycle 4: fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
}
