#include "MOS6510TestStore.h"

#include <QTest>

MOS6510TestStore::MOS6510TestStore()
{
}
MOS6510TestStore::~MOS6510TestStore()
{
}

void MOS6510TestStore::initializeRegisters()
{
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStatus(0x7D);
}
void MOS6510TestStore::verifyRegisters(quint8 accumulator, quint8 xRegister, quint8 yRegister, quint8 status)
{
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), xRegister);
    QCOMPARE(m_cpu.yRegister(), yRegister);
    QCOMPARE(m_cpu.status(), status);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestStore::testZeroPageStore_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("address");
    QTest::addColumn<quint8>("value");

    QTest::newRow("STA") << quint8(0x85) << quint8(0x20) << quint8(0x11);
    QTest::newRow("STX") << quint8(0x86) << quint8(0x20) << quint8(0x22);
    QTest::newRow("STY") << quint8(0x84) << quint8(0x20) << quint8(0x33);

    QTest::newRow("STA zero page $00") << quint8(0x85) << quint8(0x00) << quint8(0x11);
    QTest::newRow("STX zero page $00") << quint8(0x86) << quint8(0x00) << quint8(0x22);
    QTest::newRow("STY zero page $00") << quint8(0x84) << quint8(0x00) << quint8(0x33);

    QTest::newRow("STA zero page $FF") << quint8(0x85) << quint8(0xFF) << quint8(0x11);
    QTest::newRow("STX zero page $FF") << quint8(0x86) << quint8(0xFF) << quint8(0x22);
    QTest::newRow("STY zero page $FF") << quint8(0x84) << quint8(0xFF) << quint8(0x33);
}
void MOS6510TestStore::testZeroPageStore()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, address);
    QFETCH(quint8, value);

    setupCpu();
    initializeRegisters();
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, address);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(address, 0x00);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Wert schreiben
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(address), value);
    verifyRegisters(0x11, 0x22, 0x33, 0x7D);

    // Cycle 4: nächsten Opcode holen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(address), value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStore::testAbsoluteStore_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<MOS6510Operation>("operation");
    QTest::addColumn<quint16>("address");
    QTest::addColumn<quint8>("value");

    QTest::newRow("STA") << quint8(0x8D) << MOS6510Operation::STA << quint16(0x2000) << quint8(0x11);
    QTest::newRow("STX") << quint8(0x8E) << MOS6510Operation::STX << quint16(0x2000) << quint8(0x22);
    QTest::newRow("STY") << quint8(0x8C) << MOS6510Operation::STY << quint16(0x2000) << quint8(0x33);

    QTest::newRow("STA $FFFF") << quint8(0x8D) << MOS6510Operation::STA << quint16(0xFFFF) << quint8(0x11);
    QTest::newRow("STX $FFFF") << quint8(0x8E) << MOS6510Operation::STX << quint16(0xFFFF) << quint8(0x22);
    QTest::newRow("STY $FFFF") << quint8(0x8C) << MOS6510Operation::STY << quint16(0xFFFF) << quint8(0x33);
}
void MOS6510TestStore::testAbsoluteStore()
{
    QFETCH(quint8, opcode);
    QFETCH(MOS6510Operation, operation);
    QFETCH(quint16, address);
    QFETCH(quint8, value);

    setupCpu();
    initializeRegisters();
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, static_cast<quint8>(address & 0xFF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((address >> 8) & 0xFF));
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(address, 0x00);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low-Byte lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High-Byte lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Wert schreiben
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(address), value);
    verifyRegisters(0x11, 0x22, 0x33, 0x7D);

    // Cycle 5: nächsten Opcode holen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_memory.readRAM(address), value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStore::testAbsoluteYStore_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint16>("address");
    QTest::addColumn<quint8>("index");
    QTest::addColumn<quint8>("value");
    QTest::addColumn<quint16>("expectedAddress");

    QTest::newRow("STA absolute,Y") << quint8(0x99) << quint16(0x1234) << quint8(0x05) << quint8(0x11) << quint16(0x1239);
    QTest::newRow("STA absolute,Y page crossing") << quint8(0x99) << quint16(0x12F0) << quint8(0x20) << quint8(0x11) << quint16(0x1310);
    QTest::newRow("STA absolute,Y $FFFF") << quint8(0x99) << quint16(0xFFFE) << quint8(0x01) << quint8(0x11) << quint16(0xFFFF);
}
void MOS6510TestStore::testAbsoluteYStore()
{
    QFETCH(quint8, opcode);
    QFETCH(quint16, address);
    QFETCH(quint8, index);
    QFETCH(quint8, value);
    QFETCH(quint16, expectedAddress);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(index);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, static_cast<quint8>(address & 0xFF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((address >> 8) & 0xFF));
    m_memory.writeRAM(0x1003, 0xEA);
    m_memory.writeRAM(expectedAddress, 0x00);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low-Byte der Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High-Byte der Adresse lesen und Y addieren
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Wert schreiben
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(expectedAddress), value);
    verifyRegisters(0x11, 0x22, index, 0x7D);

    // Cycle 5: nächsten Opcode holen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_memory.readRAM(expectedAddress), value);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStore::testIndexedStore_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<MOS6510Operation>("operation");
    QTest::addColumn<MOS6510AddressingMode>("addressingMode");
    QTest::addColumn<quint16>("address");
    QTest::addColumn<quint8>("index");
    QTest::addColumn<quint16>("expectedAddress");

    QTest::newRow("STA zero page,X") << quint8(0x95) << MOS6510Operation::STA << MOS6510AddressingMode::ZeroPageX << quint16(0x20) << quint8(0x05) << quint16(0x25);
    QTest::newRow("STA zero page,X wrap") << quint8(0x95) << MOS6510Operation::STA << MOS6510AddressingMode::ZeroPageX << quint16(0xF0) << quint8(0x20) << quint16(0x10);
    QTest::newRow("STX zero page,Y") << quint8(0x96) << MOS6510Operation::STX << MOS6510AddressingMode::ZeroPageY << quint16(0x20) << quint8(0x05) << quint16(0x25);
    QTest::newRow("STX zero page,Y wrap") << quint8(0x96) << MOS6510Operation::STX << MOS6510AddressingMode::ZeroPageY << quint16(0xF0) << quint8(0x20) << quint16(0x10);
    QTest::newRow("STY zero page,X") << quint8(0x94) << MOS6510Operation::STY << MOS6510AddressingMode::ZeroPageX << quint16(0x20) << quint8(0x05) << quint16(0x25);
    QTest::newRow("STY zero page,X wrap") << quint8(0x94) << MOS6510Operation::STY << MOS6510AddressingMode::ZeroPageX << quint16(0xF0) << quint8(0x20) << quint16(0x10);

    QTest::newRow("STA absolute,X") << quint8(0x9D) << MOS6510Operation::STA << MOS6510AddressingMode::AbsoluteX << quint16(0x1234) << quint8(0x05) << quint16(0x1239);
    QTest::newRow("STA absolute,X page crossing") << quint8(0x9D) << MOS6510Operation::STA << MOS6510AddressingMode::AbsoluteX << quint16(0x12F0) << quint8(0x20) << quint16(0x1310);
    QTest::newRow("STA absolute,X $FFFF") << quint8(0x9D) << MOS6510Operation::STA << MOS6510AddressingMode::AbsoluteX << quint16(0xFFFE) << quint8(0x01) << quint16(0xFFFF);

    QTest::newRow("STA absolute,Y") << quint8(0x99) << MOS6510Operation::STA << MOS6510AddressingMode::AbsoluteY << quint16(0x1234) << quint8(0x05) << quint16(0x1239);
    QTest::newRow("STA absolute,Y page crossing") << quint8(0x99) << MOS6510Operation::STA << MOS6510AddressingMode::AbsoluteY << quint16(0x12F0) << quint8(0x20) << quint16(0x1310);
    QTest::newRow("STA absolute,Y $FFFF") << quint8(0x99) << MOS6510Operation::STA << MOS6510AddressingMode::AbsoluteY << quint16(0xFFFE) << quint8(0x01) << quint16(0xFFFF);
}
void MOS6510TestStore::testIndexedStore()
{
    QFETCH(quint8, opcode);
    QFETCH(MOS6510Operation, operation);
    QFETCH(MOS6510AddressingMode, addressingMode);
    QFETCH(quint16, address);
    QFETCH(quint8, index);
    QFETCH(quint16, expectedAddress);

    setupCpu();
    initializeRegisters();
    quint8 expectedValue = 0x00;
    switch (operation)
    {
    case MOS6510Operation::STA:
        expectedValue = m_cpu.accumulator();
        break;
    case MOS6510Operation::STX:
        expectedValue = m_cpu.xRegister();
        break;
    case MOS6510Operation::STY:
        expectedValue = m_cpu.yRegister();
        break;
    default:
        QFAIL("Unsupported store operation");
    }

    switch (addressingMode)
    {
    case MOS6510AddressingMode::ZeroPageX:
    case MOS6510AddressingMode::AbsoluteX:
        m_cpu.setXRegister(index);
        break;
    case MOS6510AddressingMode::ZeroPageY:
    case MOS6510AddressingMode::AbsoluteY:
        m_cpu.setYRegister(index);
        break;
    default:
        QFAIL("Unsupported addressing mode");
    }
    m_memory.writeRAM(0x1000, opcode);
    if (addressingMode == MOS6510AddressingMode::ZeroPageX ||
        addressingMode == MOS6510AddressingMode::ZeroPageY)
    {
        m_memory.writeRAM(0x1001, static_cast<quint8>(address));
        m_memory.writeRAM(0x1002, 0xEA);
    }
    else
    {
        m_memory.writeRAM(0x1001, static_cast<quint8>(address & 0xFF));
        m_memory.writeRAM(0x1002, static_cast<quint8>((address >> 8) & 0xFF));
        m_memory.writeRAM(0x1003, 0xEA);
    }
    m_memory.writeRAM(expectedAddress, 0x00);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Adresse indizieren / High-Byte lesen
    m_cpu.clock();
    if (addressingMode == MOS6510AddressingMode::ZeroPageX ||
        addressingMode == MOS6510AddressingMode::ZeroPageY)
    {
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    }
    else
    {
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    }

    // Cycle 4: Wert schreiben
    m_cpu.clock();
    QCOMPARE(m_memory.readRAM(expectedAddress), expectedValue);
    if (addressingMode == MOS6510AddressingMode::ZeroPageX ||
        addressingMode == MOS6510AddressingMode::ZeroPageY)
    {
        QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    }
    else
    {
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    }

    // Register und Status dürfen durch STORE nicht verändert werden.
    switch (operation)
    {
    case MOS6510Operation::STA:
        if (addressingMode == MOS6510AddressingMode::ZeroPageY ||
            addressingMode == MOS6510AddressingMode::AbsoluteY)
        {
            verifyRegisters(0x11, 0x22, index, 0x7D);
        }
        else
        {
            verifyRegisters(0x11, index, 0x33, 0x7D);
        }
        break;
    case MOS6510Operation::STX:
        verifyRegisters(0x11, 0x22, index, 0x7D);
        break;
    case MOS6510Operation::STY:
        verifyRegisters(0x11, index, 0x33, 0x7D);
        break;
    default:
        QFAIL("Unsupported store operation");
    }

    // Cycle 5: nächsten Opcode holen
    m_cpu.clock();
    if (addressingMode == MOS6510AddressingMode::ZeroPageX ||
        addressingMode == MOS6510AddressingMode::ZeroPageY)
    {
        QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    }
    else
    {
        QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    }
    QCOMPARE(m_memory.readRAM(expectedAddress), expectedValue);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStore::testIndexedIndirectStore_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("zeroPageAddress");
    QTest::addColumn<quint8>("index");
    QTest::addColumn<quint16>("targetAddress");

    QTest::newRow("STA (zp,X)") << quint8(0x81) << quint8(0x20) << quint8(0x05) << quint16(0x1234);
    QTest::newRow("STA (zp,X) zero page wrap") << quint8(0x81) << quint8(0xF0) << quint8(0x20) << quint16(0x1234);
    QTest::newRow("STA (zp,X) pointer $FF") << quint8(0x81) << quint8(0xFF) << quint8(0x00) << quint16(0x1234);
}
void MOS6510TestStore::testIndexedIndirectStore()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, zeroPageAddress);
    QFETCH(quint8, index);
    QFETCH(quint16, targetAddress);

    setupCpu();
    initializeRegisters();
    m_cpu.setXRegister(index);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, zeroPageAddress);
    m_memory.writeRAM(0x1002, 0xEA);
    const quint8 pointerAddress = static_cast<quint8>(zeroPageAddress + index);
    const quint8 pointerAddressHigh = static_cast<quint8>(pointerAddress + 1);
    m_memory.writeRAM(pointerAddress, static_cast<quint8>(targetAddress & 0xFF));
    m_memory.writeRAM(pointerAddressHigh, static_cast<quint8>((targetAddress >> 8) & 0xFF));
    m_memory.writeRAM(targetAddress, 0x00);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: X addieren
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 4: Pointer Low-Byte lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 5: Pointer High-Byte lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 6: Wert schreiben
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(targetAddress), quint8(0x11));
    verifyRegisters(0x11, index, 0x33, 0x7D);

    // Cycle 7: nächsten Opcode holen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(targetAddress), quint8(0x11));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestStore::testIndirectIndexedStore_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("zeroPageAddress");
    QTest::addColumn<quint8>("index");
    QTest::addColumn<quint16>("pointerAddress");
    QTest::addColumn<quint16>("expectedAddress");

    QTest::newRow("STA (zp),Y") << quint8(0x91) << quint8(0x20) << quint8(0x05) << quint16(0x1234) << quint16(0x1239);
    QTest::newRow("STA (zp),Y page crossing") << quint8(0x91) << quint8(0x20) << quint8(0x20) << quint16(0x12F0) << quint16(0x1310);
    QTest::newRow("STA (zp),Y zero page pointer wrap") << quint8(0x91) << quint8(0xFF) << quint8(0x01) << quint16(0x1234) << quint16(0x1235);
}
void MOS6510TestStore::testIndirectIndexedStore()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, zeroPageAddress);
    QFETCH(quint8, index);
    QFETCH(quint16, pointerAddress);
    QFETCH(quint16, expectedAddress);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(index);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, zeroPageAddress);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(zeroPageAddress, static_cast<quint8>(pointerAddress & 0xFF));
    const quint8 highAddress = static_cast<quint8>(zeroPageAddress + 1);
    m_memory.writeRAM(highAddress, static_cast<quint8>((pointerAddress >> 8) & 0xFF));
    m_memory.writeRAM(expectedAddress, 0x00);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Pointer-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Pointer Low-Byte lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 4: Pointer High-Byte lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 5: Wert schreiben
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_memory.readRAM(expectedAddress), quint8(0x11));
    verifyRegisters(0x11, 0x22, index, 0x7D);

    // Cycle 6: nächsten Opcode holen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_memory.readRAM(expectedAddress), quint8(0x11));
}
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------
