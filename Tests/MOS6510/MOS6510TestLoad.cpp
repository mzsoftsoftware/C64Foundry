#include "MOS6510TestLoad.h"

#include <QTest>

MOS6510TestLoad::MOS6510TestLoad()
{
}

MOS6510TestLoad::~MOS6510TestLoad()
{
}

void MOS6510TestLoad::testImmediateLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<bool>("zero");
    QTest::addColumn<bool>("negative");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << quint8(0xA9) << quint8(0x42) << false << false;
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << quint8(0xA9) << quint8(0x00) << true << false;
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << quint8(0xA9) << quint8(0x80) << false << true;

    QTest::newRow("LDX positive") << LoadRegister::X << quint8(0xA2) << quint8(0x42) << false << false;
    QTest::newRow("LDX zero") << LoadRegister::X << quint8(0xA2)<< quint8(0x00) << true << false;
    QTest::newRow("LDX negative") << LoadRegister::X << quint8(0xA2) << quint8(0x80) << false << true;

    QTest::newRow("LDY positive") << LoadRegister::Y << quint8(0xA0) << quint8(0x42) << false << false;
    QTest::newRow("LDY zero") << LoadRegister::Y << quint8(0xA0) << quint8(0x00) << true << false;
    QTest::newRow("LDY negative") << LoadRegister::Y << quint8(0xA0) << quint8(0x80) << false << true;
}
void MOS6510TestLoad::testImmediateLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(quint8, opcode);
    QFETCH(quint8, value);
    QFETCH(bool, zero);
    QFETCH(bool, negative);

    setupCpu();
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
    QCOMPARE(m_cpu.statusFlag(MOS6510StatusFlag::Zero), zero);
    QCOMPARE(m_cpu.statusFlag(MOS6510StatusFlag::Negative), negative);

    // Cycle 3: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    // Die nächste Instruktion wurde noch nicht ausgeführt.
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
    QCOMPARE(m_cpu.accumulator(), quint8(0x42));

    // Cycle 3: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), nextOperandAddress);

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    QCOMPARE(m_cpu.accumulator(), quint8(0x42));
}

void MOS6510TestLoad::testZeroPageLoad_data()
{
    QTest::addColumn<LoadRegister>("registerType");
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("address");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<bool>("zero");
    QTest::addColumn<bool>("negative");

    QTest::newRow("LDA positive") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x42) << quint8(0x37) << false << false;
    QTest::newRow("LDA zero") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x42) << quint8(0x00) << true << false;
    QTest::newRow("LDA negative") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x42) << quint8(0x80) << false << true;
    QTest::newRow("LDA $00") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0x00) << quint8(0x11) << false << false;
    QTest::newRow("LDA $FF") << LoadRegister::Accumulator << quint8(0xA5) << quint8(0xFF) << quint8(0x22) << false << false;

    QTest::newRow("LDX positive") << LoadRegister::X << quint8(0xA6) << quint8(0x42) << quint8(0x37) << false << false;
    QTest::newRow("LDX zero") << LoadRegister::X << quint8(0xA6) << quint8(0x42) << quint8(0x00) << true << false;
    QTest::newRow("LDX negative") << LoadRegister::X << quint8(0xA6) << quint8(0x42) << quint8(0x80) << false << true;
    QTest::newRow("LDX $00") << LoadRegister::X << quint8(0xA6) << quint8(0x00) << quint8(0x33) << false << false;
    QTest::newRow("LDX $FF") << LoadRegister::X << quint8(0xA6) << quint8(0xFF) << quint8(0x44) << false << false;

    QTest::newRow("LDY positive") << LoadRegister::Y << quint8(0xA4) << quint8(0x42) << quint8(0x37) << false << false;
    QTest::newRow("LDY zero") << LoadRegister::Y << quint8(0xA4) << quint8(0x42) << quint8(0x00) << true << false;
    QTest::newRow("LDY negative") << LoadRegister::Y << quint8(0xA4) << quint8(0x42) << quint8(0x80) << false << true;
    QTest::newRow("LDY $00") << LoadRegister::Y << quint8(0xA4) << quint8(0x00) << quint8(0x55) << false << false;
    QTest::newRow("LDY $FF") << LoadRegister::Y << quint8(0xA4) << quint8(0xFF) << quint8(0x66) << false << false;
}
void MOS6510TestLoad::testZeroPageLoad()
{
    QFETCH(LoadRegister, registerType);
    QFETCH(quint8, opcode);
    QFETCH(quint8, address);
    QFETCH(quint8, value);
    QFETCH(bool, zero);
    QFETCH(bool, negative);

    setupCpu();
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
    QCOMPARE(m_cpu.statusFlag(MOS6510StatusFlag::Zero), zero);
    QCOMPARE(m_cpu.statusFlag(MOS6510StatusFlag::Negative), negative);

    // Cycle 4: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
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

void MOS6510TestLoad::testZeroPageLoadPcWrap()
{
    setupCpu();
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

    // Cycle 4: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x0000));

    // Die nächste Instruktion wurde noch nicht ausgeführt.
    QCOMPARE(m_cpu.accumulator(), quint8(0x37));
}
