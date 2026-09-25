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
void MOS6510TestLogical::testLogicalAbsoluteY_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");

    QTest::newRow("AND") << quint8(0x39)
                         << quint8(0x55)
                         << quint8(0x0F)
                         << quint8(0x05)
                         << quint8(0x7D);

    QTest::newRow("ORA") << quint8(0x19)
                         << quint8(0x55)
                         << quint8(0x0A)
                         << quint8(0x5F)
                         << quint8(0x7D);

    QTest::newRow("EOR") << quint8(0x59)
                         << quint8(0x55)
                         << quint8(0x0F)
                         << quint8(0x5A)
                         << quint8(0x7D);
}

void MOS6510TestLogical::testLogicalAbsoluteY()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x02);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2342, operand);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low Byte
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High Byte + Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Operand lesen und Operation ausführen
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 5: nächster Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
void MOS6510TestLogical::testLogicalAbsoluteYPageCrossing_data()
{
    testLogicalAbsoluteY_data();
}

void MOS6510TestLogical::testLogicalAbsoluteYPageCrossing()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x02);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0x23);

    // $23FF + $02 = $2401
    m_memory.writeRAM(0x2401, operand);

    // Falsche Seite für den Dummy Read.
    m_memory.writeRAM(0x2301, 0xAA);

    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Low Byte
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: High Byte + Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));

    // Cycle 4: Dummy Read
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 5: echter Operand
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 6: nächster Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
void MOS6510TestLogical::testLogicalIndexedIndirect_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");

    QTest::newRow("AND") << quint8(0x21)
                         << quint8(0x55)
                         << quint8(0x0F)
                         << quint8(0x05)
                         << quint8(0x7D);

    QTest::newRow("ORA") << quint8(0x01)
                         << quint8(0x55)
                         << quint8(0x0A)
                         << quint8(0x5F)
                         << quint8(0x7D);

    QTest::newRow("EOR") << quint8(0x41)
                         << quint8(0x55)
                         << quint8(0x0F)
                         << quint8(0x5A)
                         << quint8(0x7D);
}

void MOS6510TestLogical::testLogicalIndexedIndirect()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x04);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    // ($20,X), X=$04 -> Pointer bei $24/$25
    m_memory.writeRAM(0x0024, 0x42);
    m_memory.writeRAM(0x0025, 0x23);

    m_memory.writeRAM(0x2342, operand);

    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page Basisadresse
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Dummy Read + X addieren
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 4: Pointer Low
    m_cpu.clock();

    // Cycle 5: Pointer High
    m_cpu.clock();

    // Cycle 6: Operand + Operation
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x04));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 7: nächster Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}

void MOS6510TestLogical::testLogicalIndexedIndirectWrapAround_data()
{
    testLogicalIndexedIndirect_data();
}

void MOS6510TestLogical::testLogicalIndexedIndirectWrapAround()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(0x01);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xEA);

    // $FE + X($01) = $FF
    // Pointer Low  bei $00FF
    // Pointer High bei $0000 -> Zero-Page-Wrap
    m_memory.writeRAM(0x00FF, 0x42);
    m_memory.writeRAM(0x0000, 0x23);

    m_memory.writeRAM(0x2342, operand);

    m_cpu.setProgramCounter(0x1000);

    m_cpu.clock();      // C1 opcode
    m_cpu.clock();      // C2 operand address
    m_cpu.clock();      // C3 dummy + X
    m_cpu.clock();      // C4 pointer low
    m_cpu.clock();      // C5 pointer high
    m_cpu.clock();      // C6 operand

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x01));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    m_cpu.clock();      // C7 next opcode
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}
void MOS6510TestLogical::testLogicalIndirectIndexed_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("status");

    QTest::newRow("AND") << quint8(0x31)
                         << quint8(0x55)
                         << quint8(0x0F)
                         << quint8(0x05)
                         << quint8(0x7D);

    QTest::newRow("ORA") << quint8(0x11)
                         << quint8(0x55)
                         << quint8(0x0A)
                         << quint8(0x5F)
                         << quint8(0x7D);

    QTest::newRow("EOR") << quint8(0x51)
                         << quint8(0x55)
                         << quint8(0x0F)
                         << quint8(0x5A)
                         << quint8(0x7D);
}

void MOS6510TestLogical::testLogicalIndirectIndexed()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x02);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    // Pointer $2340
    m_memory.writeRAM(0x0020, 0x40);
    m_memory.writeRAM(0x0021, 0x23);

    // $2340 + Y($02) = $2342
    m_memory.writeRAM(0x2342, operand);

    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    // Cycle 2: Zero-Page Pointer-Adresse
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    // Cycle 3: Pointer Low
    m_cpu.clock();

    // Cycle 4: Pointer High + Y
    m_cpu.clock();

    // Cycle 5: Operand + Operation
    m_cpu.clock();

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 6: nächster Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}

void MOS6510TestLogical::testLogicalIndirectIndexedPageCrossing_data()
{
    testLogicalIndirectIndexed_data();
}

void MOS6510TestLogical::testLogicalIndirectIndexedPageCrossing()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x02);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    // Pointer $23FF
    m_memory.writeRAM(0x0020, 0xFF);
    m_memory.writeRAM(0x0021, 0x23);

    // $23FF + $02 = $2401
    m_memory.writeRAM(0x2401, operand);

    // Adresse des Page-Crossing Dummy Reads
    m_memory.writeRAM(0x2301, 0xAA);

    m_cpu.setProgramCounter(0x1000);

    // Cycle 1
    m_cpu.clock();

    // Cycle 2
    m_cpu.clock();

    // Cycle 3: pointer low
    m_cpu.clock();

    // Cycle 4: pointer high + Y
    m_cpu.clock();

    // Cycle 5: Page-Crossing Dummy Read
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 6: Operand + Operation
    m_cpu.clock();

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    // Cycle 7: nächster Opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}

void MOS6510TestLogical::testLogicalIndirectIndexedWrapAround_data()
{
    testLogicalIndirectIndexed_data();
}

void MOS6510TestLogical::testLogicalIndirectIndexedWrapAround()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, status);

    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(0x02);
    m_cpu.setStatus(status);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0xEA);

    // Pointer Low bei $FF,
    // Pointer High muss von $00 kommen.
    m_memory.writeRAM(0x00FF, 0x40);
    m_memory.writeRAM(0x0000, 0x23);

    // $2340 + Y($02)
    m_memory.writeRAM(0x2342, operand);

    m_cpu.setProgramCounter(0x1000);

    m_cpu.clock();      // C1 opcode
    m_cpu.clock();      // C2 pointer address
    m_cpu.clock();      // C3 pointer low
    m_cpu.clock();      // C4 pointer high + Y
    m_cpu.clock();      // C5 operand

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(),
             expectedLogicalStatus(status, expectedAccumulator));

    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x02));
    QCOMPARE(m_cpu.stackPointer(), quint8(0xFF));

    m_cpu.clock();      // C6 next opcode
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
}

