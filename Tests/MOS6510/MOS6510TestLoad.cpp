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
void MOS6510TestLoad::testAbsoluteLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint16>("address");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << quint8(0xAD) << quint16(0x1234) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << quint8(0xAD) << quint16(0x1234) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << quint8(0xAD) << quint16(0x1234) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDA $0000") << LoadRegister::Accumulator << quint8(0xAD) << quint16(0x0000) << quint8(0x11) << quint8(0x7D);
    QTest::newRow("LDA $FFFF") << LoadRegister::Accumulator << quint8(0xAD) << quint16(0xFFFF) << quint8(0x22) << quint8(0x7D);

    QTest::newRow("LDX positive") << LoadRegister::X << quint8(0xAE) << quint16(0x2345) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDX zero") << LoadRegister::X << quint8(0xAE) << quint16(0x2345) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDX negative") << LoadRegister::X << quint8(0xAE) << quint16(0x2345) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDX $0000") << LoadRegister::X << quint8(0xAE) << quint16(0x0000) << quint8(0x33) << quint8(0x7D);
    QTest::newRow("LDX $FFFF") << LoadRegister::X << quint8(0xAE) << quint16(0xFFFF) << quint8(0x44) << quint8(0x7D);

    QTest::newRow("LDY positive") << LoadRegister::Y << quint8(0xAC) << quint16(0x3456) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDY zero") << LoadRegister::Y << quint8(0xAC) << quint16(0x3456) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("LDY negative") << LoadRegister::Y << quint8(0xAC) << quint16(0x3456) << quint8(0x80) << quint8(0x7D);
    QTest::newRow("LDY $0000") << LoadRegister::Y << quint8(0xAC) << quint16(0x0000) << quint8(0x55) << quint8(0x7D);
    QTest::newRow("LDY $FFFF") << LoadRegister::Y << quint8(0xAC) << quint16(0xFFFF) << quint8(0x66) << quint8(0x7D);
}
void MOS6510TestLoad::testAbsoluteLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(quint8, opcode);
    QFETCH(quint16, address);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(status);
    const quint8 lowByte = static_cast<quint8>(address & 0x00FF);
    const quint8 highByte = static_cast<quint8>((address >> 8) & 0x00FF);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, lowByte);
    m_memory.writeRAM(0x1002, highByte);
    m_memory.writeRAM(0x1003, 0xA9);
    m_memory.writeRAM(0x1004, 0x55);
    m_memory.writeRAM(address, value);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low-Byte der Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High-Byte der Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Wert aus dem Speicher lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

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

    // Cycle 5: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    verifyLoadedRegister(registerType, value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testAbsoluteIndexedLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<IndexRegister>("indexRegister");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint16>("address");
    QTest::addColumn<quint8>("indexValue");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");
    QTest::addColumn<bool>("pageCrossed");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xBD) << quint16(0x1234) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << false;
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xBD) << quint16(0x1234) << quint8(0x05) << quint8(0x00) << quint8(0x7D) << false;
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xBD) << quint16(0x1234) << quint8(0x05) << quint8(0x80) << quint8(0x7D) << false;

    QTest::newRow("LDX positive") << LoadRegister::X << IndexRegister::Y << quint8(0xBE) << quint16(0x2345) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << false;
    QTest::newRow("LDX zero") << LoadRegister::X << IndexRegister::Y << quint8(0xBE) << quint16(0x2345) << quint8(0x05) << quint8(0x00) << quint8(0x7D) << false;
    QTest::newRow("LDX negative") << LoadRegister::X << IndexRegister::Y << quint8(0xBE) << quint16(0x2345) << quint8(0x05) << quint8(0x80) << quint8(0x7D) << false;

    QTest::newRow("LDY positive") << LoadRegister::Y << IndexRegister::X << quint8(0xBC) << quint16(0x3456) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << false;
    QTest::newRow("LDY zero") << LoadRegister::Y << IndexRegister::X << quint8(0xBC) << quint16(0x3456) << quint8(0x05) << quint8(0x00) << quint8(0x7D) << false;
    QTest::newRow("LDY negative") << LoadRegister::Y << IndexRegister::X << quint8(0xBC) << quint16(0x3456) << quint8(0x05) << quint8(0x80) << quint8(0x7D) << false;

    QTest::newRow("LDA page crossing") << LoadRegister::Accumulator << IndexRegister::X << quint8(0xBD) << quint16(0x12FE) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << true;
    QTest::newRow("LDX page crossing") << LoadRegister::X << IndexRegister::Y << quint8(0xBE) << quint16(0x23FE) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << true;
    QTest::newRow("LDY page crossing") << LoadRegister::Y << IndexRegister::X << quint8(0xBC) << quint16(0x34FE) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << true;
}
void MOS6510TestLoad::testAbsoluteIndexedLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(IndexRegister, indexRegister);
    QFETCH(quint8, opcode);
    QFETCH(quint16, address);
    QFETCH(quint8, indexValue);
    QFETCH(quint8, value);
    QFETCH(quint8, status);
    QFETCH(bool, pageCrossed);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(status);
    switch (indexRegister)
    {
    case IndexRegister::X:
        m_cpu.setXRegister(indexValue);
        break;
    case IndexRegister::Y:
        m_cpu.setYRegister(indexValue);
        break;
    }
    const quint16 effectiveAddress = address + indexValue;
    const quint8 lowByte = static_cast<quint8>(address & 0x00FF);
    const quint8 highByte = static_cast<quint8>((address >> 8) & 0x00FF);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, lowByte);
    m_memory.writeRAM(0x1002, highByte);
    m_memory.writeRAM(0x1003, 0xA9);
    m_memory.writeRAM(0x1004, 0x55);
    m_memory.writeRAM(effectiveAddress, value);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low-Byte der Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High-Byte lesen und Index addieren
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    if (pageCrossed)
    {
        // Cycle 4: zusätzlicher Zyklus durch Page Crossing
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

        // Der Zielwert darf zu diesem Zeitpunkt noch nicht geladen sein.
        switch (registerType)
        {
        case LoadRegister::Accumulator:
            QCOMPARE(m_cpu.accumulator(), quint8(0x11));
            break;
        case LoadRegister::X:
            QCOMPARE(m_cpu.xRegister(), quint8(0x22));
            break;
        case LoadRegister::Y:
            QCOMPARE(m_cpu.yRegister(), quint8(0x33));
            break;
        }
    }

    // Cycle 4: Wert aus dem Speicher lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    verifyLoadedRegister(registerType, value);
    QCOMPARE(m_cpu.status(), expectedLoadStatus(status, value));

    switch (registerType)
    {
    case LoadRegister::Accumulator:
        QCOMPARE(m_cpu.xRegister(), indexValue);
        QCOMPARE(m_cpu.yRegister(), quint8(0x33));
        break;
    case LoadRegister::X:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.yRegister(), indexValue);
        break;
    case LoadRegister::Y:
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        QCOMPARE(m_cpu.xRegister(), indexValue);
        break;
    }

    // Cycle 5: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    verifyLoadedRegister(registerType, value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testAbsoluteYLoad_data()
{
    QTest::addColumn<quint16>("address");
    QTest::addColumn<quint8>("indexValue");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");
    QTest::addColumn<bool>("pageCrossed");

    QTest::newRow("positive") << quint16(0x1234) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << false;
    QTest::newRow("zero") << quint16(0x2340) << quint8(0x05) << quint8(0x00) << quint8(0x7D) << false;
    QTest::newRow("negative") << quint16(0x3450) << quint8(0x05) << quint8(0x80) << quint8(0x7D) << false;
    QTest::newRow("page crossing") << quint16(0x12FE) << quint8(0x05) << quint8(0x37) << quint8(0x7D) << true;
}
void MOS6510TestLoad::testAbsoluteYLoad()
{
    QFETCH(quint16, address);
    QFETCH(quint8, indexValue);
    QFETCH(quint8, value);
    QFETCH(quint8, status);
    QFETCH(bool, pageCrossed);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(indexValue);
    m_cpu.setStatus(status);
    m_memory.writeRAM(address + indexValue, value);
    m_memory.writeRAM(0x1000, 0xB9);
    m_memory.writeRAM(0x1001, static_cast<quint8>(address & 0xFF));
    m_memory.writeRAM(0x1002, static_cast<quint8>(address >> 8));
    m_memory.writeRAM(0x1003, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Address Low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Address High + Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    if (pageCrossed)
    {
        // Cycle 4: Dummy Read
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

        // Accumulator must not be loaded yet
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    }

    // Cycle 4: Data
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), value);
    QCOMPARE(m_cpu.status(), expectedLoadStatus(status, value));

    // Cycle 5: Next Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLoad::testIndirectLoad_data()
{
    QTest::addColumn<IndirectLoadMode>("mode");
    QTest::addColumn<quint8>("zeroPageAddress");
    QTest::addColumn<quint8>("indexValue");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint8>("status");

    QTest::newRow("LDA indexed indirect") << IndirectLoadMode::IndexedIndirect << quint8(0x20) << quint8(0x05) << quint16(0x1234) << quint16(0x1234) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA indirect indexed") << IndirectLoadMode::IndirectIndexed << quint8(0x20) << quint8(0x05) << quint16(0x122F) << quint16(0x1234) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA indexed indirect zero page wrap") << IndirectLoadMode::IndexedIndirect << quint8(0xFE) << quint8(0x05) << quint16(0x1234) << quint16(0x1234) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA indexed indirect pointer wrap") << IndirectLoadMode::IndexedIndirect << quint8(0xFE) << quint8(0x01) << quint16(0x1234) << quint16(0x1234) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA indirect indexed pointer wrap") << IndirectLoadMode::IndirectIndexed << quint8(0xFF) << quint8(0x00) << quint16(0x1234) << quint16(0x1234) << quint8(0x37) << quint8(0x7D);
    QTest::newRow("LDA indirect indexed page crossing") << IndirectLoadMode::IndirectIndexed << quint8(0x20) << quint8(0x01) << quint16(0x12FF) << quint16(0x1300) << quint8(0x37) << quint8(0x7D);
}

void MOS6510TestLoad::testIndirectLoad()
{
    QFETCH(IndirectLoadMode, mode);
    QFETCH(quint8, zeroPageAddress);
    QFETCH(quint8, indexValue);
    QFETCH(quint16, baseAddress);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, value);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(status);
    switch (mode)
    {
    case IndirectLoadMode::IndexedIndirect:
        m_cpu.setXRegister(indexValue);
        break;
    case IndirectLoadMode::IndirectIndexed:
        m_cpu.setYRegister(indexValue);
        break;
    }
    m_memory.writeRAM(targetAddress, value);
    switch (mode)
    {
    case IndirectLoadMode::IndexedIndirect:
    {
        const quint8 pointerAddress = static_cast<quint8>(zeroPageAddress + indexValue);
        m_memory.writeRAM(pointerAddress, static_cast<quint8>(targetAddress & 0xFF));
        m_memory.writeRAM(static_cast<quint8>(pointerAddress + 1), static_cast<quint8>(targetAddress >> 8));
        m_memory.writeRAM(0x1000, 0xA1);
        break;
    }
    case IndirectLoadMode::IndirectIndexed:
        m_memory.writeRAM(zeroPageAddress, static_cast<quint8>(baseAddress & 0xFF));
        m_memory.writeRAM(static_cast<quint8>(zeroPageAddress + 1), static_cast<quint8>(baseAddress >> 8));
        m_memory.writeRAM(0x1000, 0xB1);
        break;
    }
    m_memory.writeRAM(0x1001, zeroPageAddress);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    switch (mode)
    {
    case IndirectLoadMode::IndexedIndirect:
        // Cycle 1: Opcode Fetch
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

        // Cycle 2: Zero-Page Operand
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

        // Cycle 3: Zero-Page + X
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));

        // Cycle 4: Pointer Low
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

        // Cycle 5: Pointer High
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

        // Cycle 6: Data
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
        QCOMPARE(m_cpu.accumulator(), value);
        QCOMPARE(m_cpu.status(), expectedLoadStatus(status, value));

        // Cycle 7: Next Opcode Fetch
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
        QCOMPARE(m_cpu.accumulator(), value);
        break;

    case IndirectLoadMode::IndirectIndexed:
        // Cycle 1: Opcode Fetch
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

        // Cycle 2: Zero-Page Pointer Address
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

        // Cycle 3: Pointer Low
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

        // Cycle 4: Pointer High + Y
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
        QCOMPARE(m_cpu.accumulator(), quint8(0x11));

        if ((baseAddress & 0xFF00) != (targetAddress & 0xFF00))
        {
            // Cycle 5: Dummy Read
            m_cpu.clock();
            QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
            QCOMPARE(m_cpu.accumulator(), quint8(0x11));
        }

        // Cycle 6: Data
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
        QCOMPARE(m_cpu.accumulator(), value);
        QCOMPARE(m_cpu.status(), expectedLoadStatus(status, value));

        // Cycle 7: Next Opcode Fetch
        m_cpu.clock();
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
        QCOMPARE(m_cpu.accumulator(), value);
        break;
    }
}
