#include "MOS6510TestLoad.h"

#include <QTest>

MOS6510TestLoad::MOS6510TestLoad()
{
}

MOS6510TestLoad::~MOS6510TestLoad()
{
}

void MOS6510TestLoad::initializeRegisters()
{
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
}
void MOS6510TestLoad::verifyLoadedRegister(const LoadRegister registerType, const quint8 value)
{
    switch (registerType)
    {
    case LoadRegister::Accumulator:
        QCOMPARE(m_cpu.accumulator(), value);
        break;
    case LoadRegister::X:
        QCOMPARE(m_cpu.xRegister(), value);
        break;
    case LoadRegister::Y:
        QCOMPARE(m_cpu.yRegister(), value);
        break;
    }
}
quint8 MOS6510TestLoad::expectedLoadStatus(const quint8 status, const quint8 value) const
{
    quint8 result = status;
    result &= static_cast<quint8>(~static_cast<quint8>(MOS6510StatusFlag::Zero));
    result &= static_cast<quint8>(~static_cast<quint8>(MOS6510StatusFlag::Negative));
    if (value == 0)
    {
        result |= static_cast<quint8>(MOS6510StatusFlag::Zero);
    }
    if ((value & 0x80) != 0)
    {
        result |= static_cast<quint8>(MOS6510StatusFlag::Negative);
    }
    return result;
}

// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testImmediateLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << quint8(0xA9) << quint8(0x42) << quint8(0x45);
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << quint8(0xA9) << quint8(0x00) << quint8(0x45);
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << quint8(0xA9) << quint8(0x80) << quint8(0x45);

    QTest::newRow("LDX positive") << LoadRegister::X << quint8(0xA2) << quint8(0x42) << quint8(0x45);
    QTest::newRow("LDX zero") << LoadRegister::X << quint8(0xA2) << quint8(0x00) << quint8(0x45);
    QTest::newRow("LDX negative") << LoadRegister::X << quint8(0xA2) << quint8(0x80) << quint8(0x45);

    QTest::newRow("LDY positive") << LoadRegister::Y << quint8(0xA0) << quint8(0x42) << quint8(0x45);
    QTest::newRow("LDY zero") << LoadRegister::Y << quint8(0xA0) << quint8(0x00) << quint8(0x45);
    QTest::newRow("LDY negative") << LoadRegister::Y << quint8(0xA0) << quint8(0x80) << quint8(0x45);
}
void MOS6510TestLoad::testImmediateLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(quint8, opcode);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, value);
    m_memory.writeRAM(0x1002, 0xA9);
    m_memory.writeRAM(0x1003, 0x55);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Immediate-Wert lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    verifyLoadedRegister(registerType, value);
    QCOMPARE(m_cpu.status(),
             expectedLoadStatus(status, value));

    // Die beiden anderen Register wurden nicht verändert.
    switch (registerType)
    {
    case LoadRegister::Accumulator:
        QCOMPARE(m_cpu.xRegister(), quint8(0x22));
        QCOMPARE(m_cpu.yRegister(), quint8(0x33));
        break;

    case LoadRegister::X:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.yRegister(), quint8(0x33));
        break;

    case LoadRegister::Y:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.xRegister(), quint8(0x22));
        break;
    }

    // Cycle 3: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    verifyLoadedRegister(registerType, value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testImmediateLoadPcWrap_data()
{
    QTest::addColumn<quint16>("startAddress");
    QTest::addColumn<quint16>("operandAddress");
    QTest::addColumn<quint16>("nextOpcodeAddress");
    QTest::addColumn<quint16>("nextOperandAddress");

    QTest::newRow("LDA at $FFFE") << quint16(0xFFFE) << quint16(0xFFFF) << quint16(0x0000) << quint16(0x0001);
    QTest::newRow("LDA at $FFFF") << quint16(0xFFFF) << quint16(0x0000) << quint16(0x0001) << quint16(0x0002);
}
void MOS6510TestLoad::testImmediateLoadPcWrap()
{
    QFETCH(quint16, startAddress);
    QFETCH(quint16, operandAddress);
    QFETCH(quint16, nextOpcodeAddress);
    QFETCH(quint16, nextOperandAddress);

    setupCpu();
    initializeRegisters();
    const quint8 initialStatus = 0x7D;
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(startAddress, 0xA9);
    m_memory.writeRAM(operandAddress, 0x42);
    m_memory.writeRAM(nextOpcodeAddress, 0xA9);
    m_memory.writeRAM(nextOperandAddress, 0x55);
    m_cpu.setProgramCounter(startAddress);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), operandAddress);

    // Cycle 2: Immediate-Wert lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), nextOpcodeAddress);

    verifyLoadedRegister(LoadRegister::Accumulator, 0x42);
    QCOMPARE(m_cpu.status(),
             expectedLoadStatus(initialStatus, 0x42));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    QCOMPARE(m_cpu.accumulator(), quint8(0x42));

    // Cycle 3: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), nextOperandAddress);

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testZeroPageLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("address");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x42) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x42) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x42) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDA $00") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x00) << quint8(0x11) << quint8(0x7D);
    QTest::newRow("LDA $FF") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0xFF) << quint8(0x22) << quint8(0x7D);

    QTest::newRow("LDX positive") << LoadRegister::X << quint8(0xA6) << quint8(0x42) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDX zero") << LoadRegister::X << quint8(0xA6) << quint8(0x42) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDX negative") << LoadRegister::X << quint8(0xA6) << quint8(0x42) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDX $00") << LoadRegister::X << quint8(0xA6) << quint8(0x00) << quint8(0x33) << quint8(0x7D);
    QTest::newRow("LDX $FF") << LoadRegister::X << quint8(0xA6) << quint8(0xFF) << quint8(0x44) << quint8(0x7D);

    QTest::newRow("LDY positive") << LoadRegister::Y << quint8(0xA4) << quint8(0x42) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDY zero") << LoadRegister::Y << quint8(0xA4) << quint8(0x42) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDY negative") << LoadRegister::Y << quint8(0xA4) << quint8(0x42) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDY $00") << LoadRegister::Y << quint8(0xA4) << quint8(0x00) << quint8(0x55) << quint8(0x7D);
    QTest::newRow("LDY $FF") << LoadRegister::Y << quint8(0xA4) << quint8(0xFF) << quint8(0x66) << quint8(0x7D);
}
void MOS6510TestLoad::testZeroPageLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(quint8, opcode);
    QFETCH(quint8, address);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, address);
    m_memory.writeRAM(0x1002, 0xA9);
    m_memory.writeRAM(0x1003, 0x55);
    m_memory.writeRAM(address, value);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Wert aus Zero Page lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    verifyLoadedRegister(registerType, value);
    QCOMPARE(m_cpu.status(),
             expectedLoadStatus(status, value));

    // Die beiden anderen Register wurden nicht verändert.
    switch (registerType)
    {
    case LoadRegister::Accumulator:
        QCOMPARE(m_cpu.xRegister(), quint8(0x22));
        QCOMPARE(m_cpu.yRegister(), quint8(0x33));
        break;

    case LoadRegister::X:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.yRegister(), quint8(0x33));
        break;

    case LoadRegister::Y:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.xRegister(), quint8(0x22));
        break;
    }

    // Cycle 4: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    verifyLoadedRegister(registerType, value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testZeroPageLoadPcWrap()
{
    setupCpu();
    initializeRegisters();
    const quint8 initialStatus = 0x7D;
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0xFFFD, 0xA5);
    m_memory.writeRAM(0xFFFE, 0x42);
    m_memory.writeRAM(0xFFFF, 0xA9);
    m_memory.writeRAM(0x0000, 0x55);
    m_memory.writeRAM(0x0042, 0x37);
    m_cpu.setProgramCounter(0xFFFD);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0xFFFE));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0xFFFF));

    // Cycle 3: Wert aus Zero Page lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0xFFFF));

    QCOMPARE(m_cpu.accumulator(), quint8(0x37));
    QCOMPARE(m_cpu.status(), expectedLoadStatus(initialStatus, 0x37));

    // X und Y wurden nicht verändert.
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));

    // Cycle 4: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x0000));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    QCOMPARE(m_cpu.accumulator(), quint8(0x37));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testZeroPageIndexedLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<IndexRegister>("indexRegister");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("address");
    QTest::addColumn<quint8>("index");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xB5) << quint8(0x40) << quint8(0x02) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xB5) << quint8(0x40) << quint8(0x02) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xB5) << quint8(0x40) << quint8(0x02) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDA zero-page wrap") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xB5) << quint8(0xFF) << quint8(0x01) << quint8(0x42) << quint8(0x7D);

    QTest::newRow("LDX positive") << LoadRegister::X << IndexRegister::Y << quint8(0xB6) << quint8(0x40) << quint8(0x02) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDX zero") << LoadRegister::X << IndexRegister::Y << quint8(0xB6) << quint8(0x40) << quint8(0x02) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDX negative") << LoadRegister::X << IndexRegister::Y << quint8(0xB6) << quint8(0x40) << quint8(0x02) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDX zero-page wrap") << LoadRegister::X << IndexRegister::Y << quint8(0xB6) << quint8(0xFF) << quint8(0x01) << quint8(0x43) << quint8(0x7D);

    QTest::newRow("LDY positive") << LoadRegister::Y << IndexRegister::X << quint8(0xB4) << quint8(0x40) << quint8(0x02) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDY zero") << LoadRegister::Y << IndexRegister::X << quint8(0xB4) << quint8(0x40) << quint8(0x02) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDY negative") << LoadRegister::Y << IndexRegister::X << quint8(0xB4) << quint8(0x40) << quint8(0x02) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDY zero-page wrap") << LoadRegister::Y << IndexRegister::X << quint8(0xB4) << quint8(0xFF) << quint8(0x01) << quint8(0x44) << quint8(0x7D);
}
void MOS6510TestLoad::testZeroPageIndexedLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(IndexRegister, indexRegister);
    QFETCH(quint8, opcode);
    QFETCH(quint8, address);
    QFETCH(quint8, index);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(status);
    switch (indexRegister)
    {
    case IndexRegister::X:
        m_cpu.setXRegister(index);
        break;

    case IndexRegister::Y:
        m_cpu.setYRegister(index);
        break;
    }
    const quint8 effectiveAddress = static_cast<quint8>(address + index);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, address);
    m_memory.writeRAM(0x1002, 0xA9);
    m_memory.writeRAM(0x1003, 0x55);
    m_memory.writeRAM(effectiveAddress, value);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Dummy Read / Indexierung
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 4: Wert aus der indizierten Zero Page lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    verifyLoadedRegister(registerType, value);
    QCOMPARE(m_cpu.status(),
             expectedLoadStatus(status, value));

    // Die beiden anderen Register wurden nicht verändert.
    switch (registerType)
    {
    case LoadRegister::Accumulator:
        QCOMPARE(m_cpu.xRegister(), indexRegister == IndexRegister::X ? index : quint8(0x22));
        QCOMPARE(m_cpu.yRegister(), indexRegister == IndexRegister::Y ? index : quint8(0x33));
        break;

    case LoadRegister::X:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.yRegister(), indexRegister == IndexRegister::Y ? index : quint8(0x33));
        break;

    case LoadRegister::Y:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.xRegister(), indexRegister == IndexRegister::X ? index : quint8(0x22));
        break;
    }

    // Cycle 5: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    verifyLoadedRegister(registerType, value);
}
// --------------------------------------------------------------------------------------------
