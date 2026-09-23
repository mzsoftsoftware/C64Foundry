#include "MOS6510TestLogical.h"

#include <QTest>


MOS6510TestLogical::MOS6510TestLogical()
{
}
MOS6510TestLogical::~MOS6510TestLogical()
{
}

void MOS6510TestLogical::initializeRegisters()
{
    m_cpu.setAccumulator(0x55);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x7D);
}
quint8 MOS6510TestLogical::expectedLogicalStatus(const quint8 status, const quint8 value) const
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
void MOS6510TestLogical::testLogicalImmediate_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("AND positive") << quint8(0x29) << quint8(0x55) << quint8(0x0F) << quint8(0x05) << quint8(0x7D);
    QTest::newRow("AND zero") << quint8(0x29) << quint8(0x55) << quint8(0xAA) << quint8(0x00) << quint8(0x7F);
    QTest::newRow("AND negative") << quint8(0x29) << quint8(0xFF) << quint8(0x80) << quint8(0x80) << quint8(0xFD);

    QTest::newRow("ORA positive") << quint8(0x09) << quint8(0x55) << quint8(0x0A) << quint8(0x5F) << quint8(0x7D);
    QTest::newRow("ORA zero") << quint8(0x09) << quint8(0x00) << quint8(0x00) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("ORA negative") << quint8(0x09) << quint8(0x00) << quint8(0x80) << quint8(0x80) << quint8(0x7D);

    QTest::newRow("EOR positive") << quint8(0x49) << quint8(0x55) << quint8(0x0F) << quint8(0x5A) << quint8(0x7D);
    QTest::newRow("EOR zero") << quint8(0x49) << quint8(0x00) << quint8(0x00) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("EOR negative") << quint8(0x49) << quint8(0x00) << quint8(0x80) << quint8(0x80) << quint8(0x7D);

}
void MOS6510TestLogical::testLogicalImmediate()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(expectedStatus);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Immediate-Wert lesen und AND ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedLogicalStatus(expectedStatus, expectedAccumulator));
    // X und Y wurden nicht verändert.
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 3: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    // Die nächste Instruktion wurde noch nicht ausgeführt.
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLogical::testLogicalZeroPage_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");

    QTest::newRow("AND positive") << quint8(0x25) << quint8(0x55) << quint8(0x0F) << quint8(0x05) << quint8(0x7D);
    QTest::newRow("AND zero") << quint8(0x25) << quint8(0x00) << quint8(0x00) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("AND negative") << quint8(0x25) << quint8(0x00) << quint8(0x80) << quint8(0x00) << quint8(0x7D);

    QTest::newRow("ORA positive") << quint8(0x05) << quint8(0x55) << quint8(0x0A) << quint8(0x5F) << quint8(0x7D);
    QTest::newRow("ORA zero") << quint8(0x05) << quint8(0x00) << quint8(0x00) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("ORA negative") << quint8(0x05) << quint8(0x00) << quint8(0x80) << quint8(0x80) << quint8(0x7D);

    QTest::newRow("EOR positive") << quint8(0x45) << quint8(0x55) << quint8(0x0F) << quint8(0x5A) << quint8(0x7D);
    QTest::newRow("EOR zero") << quint8(0x45) << quint8(0x00) << quint8(0x00) << quint8(0x00) << quint8(0x7D);
    QTest::newRow("EOR negative") << quint8(0x45) << quint8(0x00) << quint8(0x80) << quint8(0x80) << quint8(0x7D);
}
void MOS6510TestLogical::testLogicalZeroPage()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);
    m_memory.writeRAM(0x0042, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Operand lesen und Operation ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedLogicalStatus(status, expectedAccumulator));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 4: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLogical::testLogicalZeroPageIndexed_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");
    QTest::addColumn<quint8>("baseAddress");
    QTest::addColumn<quint8>("address");

    QTest::newRow("AND positive") << quint8(0x35) << quint8(0x55) << quint8(0x0F) << quint8(0x02) << quint8(0x05) << quint8(0x7D) << quint8(0x41) << quint8(0x43);
    QTest::newRow("ORA positive") << quint8(0x15) << quint8(0x55) << quint8(0x0A) << quint8(0x02) << quint8(0x5F) << quint8(0x7D) << quint8(0x41) << quint8(0x43);
    QTest::newRow("EOR positive") << quint8(0x55) << quint8(0x55) << quint8(0x0F) << quint8(0x02) << quint8(0x5A) << quint8(0x7D) << quint8(0x41) << quint8(0x43);

    QTest::newRow("AND zero page wrap") << quint8(0x35) << quint8(0xFF) << quint8(0xFF) << quint8(0x01) << quint8(0xFF) << quint8(0x7D) << quint8(0xFF) << quint8(0x00);
    QTest::newRow("ORA zero page wrap") << quint8(0x15) << quint8(0x00) << quint8(0x80) << quint8(0x01) << quint8(0x80) << quint8(0x7D) << quint8(0xFF) << quint8(0x00);
    QTest::newRow("EOR zero page wrap") << quint8(0x55) << quint8(0x80) << quint8(0x80) << quint8(0x01) << quint8(0x00) << quint8(0x7D) << quint8(0xFF) << quint8(0x00);
}
void MOS6510TestLogical::testLogicalZeroPageIndexed()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, xRegister);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);
    QFETCH(quint8, baseAddress);
    QFETCH(quint8, address);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(xRegister);
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, baseAddress);
    m_memory.writeRAM(address, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page-Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Zero-Page-Adresse + X berechnen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 4: Operand lesen und Operation ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedLogicalStatus(status, expectedAccumulator));
    QCOMPARE(m_cpu.xRegister(), xRegister);
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 5: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLogical::testLogicalAbsolute_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");
    QTest::addColumn<quint16>("address");

    QTest::newRow("AND positive") << quint8(0x2D) << quint8(0x55) << quint8(0x0F) << quint8(0x05) << quint8(0x7D) << quint16(0x2342);
    QTest::newRow("ORA positive") << quint8(0x0D) << quint8(0x55) << quint8(0x0A) << quint8(0x5F) << quint8(0x7D) << quint16(0x2342);
    QTest::newRow("EOR positive") << quint8(0x4D) << quint8(0x55) << quint8(0x0F) << quint8(0x5A) << quint8(0x7D) << quint16(0x2342);

    QTest::newRow("AND zero") << quint8(0x2D) << quint8(0x55) << quint8(0xAA) << quint8(0x00) << quint8(0x7D) << quint16(0x2342);
    QTest::newRow("ORA negative") << quint8(0x0D) << quint8(0x00) << quint8(0x80) << quint8(0x80) << quint8(0x7D) << quint16(0x2342);
    QTest::newRow("EOR negative") << quint8(0x4D) << quint8(0xFF) << quint8(0x80) << quint8(0x7F) << quint8(0x7D) << quint16(0x2342);
}
void MOS6510TestLogical::testLogicalAbsolute()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);
    QFETCH(quint16, address);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, static_cast<quint8>(address & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>(address >> 8));
    m_memory.writeRAM(address, operand);
    m_memory.writeRAM(0x1003, 0xEA);
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

    // Cycle 4: Operand lesen und Operation ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedLogicalStatus(status, expectedAccumulator));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 5: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLogical::testLogicalAbsoluteIndexed_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");
    QTest::addColumn<quint8>("xRegister");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint16>("address");

    QTest::newRow("AND positive") << quint8(0x3D) << quint8(0x55) << quint8(0x0F) << quint8(0x05) << quint8(0x7D) << quint8(0x02) << quint16(0x2340) << quint16(0x2342);
    QTest::newRow("ORA positive") << quint8(0x1D) << quint8(0x55) << quint8(0x0A) << quint8(0x5F) << quint8(0x7D) << quint8(0x02) << quint16(0x2340) << quint16(0x2342);
    QTest::newRow("EOR positive") << quint8(0x5D) << quint8(0x55) << quint8(0x0F) << quint8(0x5A) << quint8(0x7D) << quint8(0x02) << quint16(0x2340) << quint16(0x2342);

    QTest::newRow("AND zero") << quint8(0x3D) << quint8(0x55) << quint8(0xAA) << quint8(0x00) << quint8(0x7D) << quint8(0x02) << quint16(0x2340) << quint16(0x2342);
    QTest::newRow("ORA negative") << quint8(0x1D) << quint8(0x00) << quint8(0x80) << quint8(0x80) << quint8(0x7D) << quint8(0x02) << quint16(0x2340) << quint16(0x2342);
    QTest::newRow("EOR negative") << quint8(0x5D) << quint8(0xFF) << quint8(0x80) << quint8(0x7F) << quint8(0x7D) << quint8(0x02) << quint16(0x2340) << quint16(0x2342);
}
void MOS6510TestLogical::testLogicalAbsoluteIndexed()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);
    QFETCH(quint8, xRegister);
    QFETCH(quint16, baseAddress);
    QFETCH(quint16, address);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(xRegister);
    m_cpu.setStatus(status);
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>(baseAddress >> 8));
    m_memory.writeRAM(address, operand);
    m_memory.writeRAM(0x1003, 0xEA);
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

    // Cycle 4: X addieren, Operand lesen und Operation ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedLogicalStatus(status, expectedAccumulator));
    QCOMPARE(m_cpu.xRegister(), xRegister);
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 5: Opcode der nächsten Instruktion lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestLogical::testLogicalAbsoluteIndexedPageCrossing_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");

    QTest::newRow("AND") << quint8(0x3D) << quint8(0x55) << quint8(0x0F) << quint8(0x05) << quint8(0x7D);
    QTest::newRow("ORA") << quint8(0x1D) << quint8(0x55) << quint8(0x0A) << quint8(0x5F) << quint8(0x7D);
    QTest::newRow("EOR") << quint8(0x5D) << quint8(0x55) << quint8(0x0F) << quint8(0x5A) << quint8(0x7D);
}
void MOS6510TestLogical::testLogicalAbsoluteIndexedPageCrossing()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x02);
    m_cpu.setStatus(status);
    const quint16 baseAddress = 0x23FF;
    const quint16 address = 0x2401;
    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>(baseAddress >> 8));
    // Wert an der tatsächlich erwarteten Adresse
    m_memory.writeRAM(address, operand);
    // Wert an der falschen Adresse, damit wir den Dummy Read
    // vom eigentlichen Operand Read unterscheiden können.
    m_memory.writeRAM(0x2301, 0xAA);
    // Nächster Opcode
    m_memory.writeRAM(0x1003, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low-Byte der Adresse lesen
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High-Byte lesen und X addieren
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Dummy Read wegen Page Crossing
    m_cpu.clock();
    // Operation darf hier noch NICHT ausgeführt worden sein.
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 5: Operand lesen und Operation ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedLogicalStatus(status, expectedAccumulator));
    QCOMPARE(m_cpu.xRegister(), quint8(0x02));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 6: nächster Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
// --------------------------------------------------------------------------------------------
