#include "MOS6510TestArithmetic.h"

#include <QTest>


MOS6510TestArithmetic::MOS6510TestArithmetic()
{
}
MOS6510TestArithmetic::~MOS6510TestArithmetic()
{
}

void MOS6510TestArithmetic::initializeRegisters()
{
    m_cpu.setAccumulator(0x55);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0xFF);
    m_cpu.setStatus(0x7D);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x10) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34);
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34);
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4);
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77);
}
void MOS6510TestArithmetic::testAdcImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(static_cast<quint8>(0x74 | carryIn));
    m_memory.writeRAM(0x1000, 0x69);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: ADC immediate
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcZeroPage_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x10) << quint8(0x20) << quint8(0x00)<< quint8(0x30) << quint8(0x34);
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34);
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4);
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77);
}
void MOS6510TestArithmetic::testAdcZeroPage()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x65);  // ADC Zero Page
    m_memory.writeRAM(0x1001, 0x20);  // Zero Page address
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    m_memory.writeRAM(0x0020, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 3: Read operand and ADC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcZeroPageX_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x10) << quint8(0x20) << quint8(0x05) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34);
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x05) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34);
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x20) << quint8(0x05) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x20) << quint8(0x05) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x20) << quint8(0x05) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4);
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x20) << quint8(0x05) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77);
    QTest::newRow("zero page wraparound") << quint8(0x10) << quint8(0xF0) << quint8(0x20) << quint8(0x05) << quint8(0x00) << quint8(0x15) << quint8(0x34);
}

void MOS6510TestArithmetic::testAdcZeroPageX()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x75);  // ADC Zero Page,X
    m_memory.writeRAM(0x1001, baseAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 effectiveAddress = static_cast<quint8>(baseAddress + x);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read Zero Page base address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Add X to Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read operand and ADC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcAbsolute_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x10) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34);
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34);
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4);
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77);
}

void MOS6510TestArithmetic::testAdcAbsolute()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x6D);  // ADC Absolute
    m_memory.writeRAM(0x1001, 0x45);  // Low byte
    m_memory.writeRAM(0x1002, 0x23);  // High byte
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    m_memory.writeRAM(0x2345, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 3: Read address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 4: Read operand and ADC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcAbsoluteX_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x10) << quint16(0x2340) << quint8(0x05) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << false;
    QTest::newRow("with page crossing") << quint8(0x10) << quint16(0x23F0) << quint8(0x20) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << true;
}
void MOS6510TestArithmetic::testAdcAbsoluteX()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x7D);  // ADC Absolute,X
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((baseAddress >> 8) & 0x00FF));
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    const quint16 effectiveAddress = static_cast<quint16>(baseAddress + x);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Read address high + add X
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    if (pageCrossing)
    {
        // Cycle 4: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.status(), initialStatus);

        // Cycle 5: Read operand and ADC
        m_cpu.clock();
    }
    else
    {
        // Cycle 4: Read operand and ADC
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcAbsoluteY_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x10) << quint16(0x2340) << quint8(0x05) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << false;
    QTest::newRow("with page crossing") << quint8(0x10) << quint16(0x23F0) << quint8(0x20) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << true;
}
void MOS6510TestArithmetic::testAdcAbsoluteY()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, y);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(y);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x79);  // ADC Absolute,Y
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((baseAddress >> 8) & 0x00FF));
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    const quint16 effectiveAddress = static_cast<quint16>(baseAddress + y);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read address high + add Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    if (pageCrossing)
    {
        // Cycle 4: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.yRegister(), y);
        QCOMPARE(m_cpu.status(), initialStatus);

        // Cycle 5: Read operand and ADC
        m_cpu.clock();
    }
    else
    {
        // Cycle 4: Read operand and ADC
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcIndexedIndirect_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("pointerAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x10) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34);
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34);
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4);
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77);
    QTest::newRow("zero page pointer wraparound") << quint8(0x10) << quint8(0xFC) << quint8(0x04) << quint16(0x2345) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34);
}
void MOS6510TestArithmetic::testAdcIndexedIndirect()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, pointerAddress);
    QFETCH(quint8, x);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x61);  // ADC (Zero Page,X)
    m_memory.writeRAM(0x1001, pointerAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 effectivePointer = static_cast<quint8>(pointerAddress + x);
    m_memory.writeRAM(effectivePointer, static_cast<quint8>(targetAddress & 0x00FF));
    m_memory.writeRAM(static_cast<quint8>(effectivePointer + 1), static_cast<quint8>((targetAddress >> 8) & 0x00FF));
    m_memory.writeRAM(targetAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Add X to Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read target address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 5: Read target address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 6: Read operand and ADC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 7: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testAdcIndirectIndexed_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("pointerAddress");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x10) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << false;
    QTest::newRow("carry in") << quint8(0x10) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x20) << quint8(0x01) << quint8(0x31) << quint8(0x34) << false;
    QTest::newRow("zero and carry") << quint8(0xFF) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x01) << quint8(0x00) << quint8(0x00) << quint8(0x37) << false;
    QTest::newRow("negative and overflow") << quint8(0x40) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x40) << quint8(0x00) << quint8(0x80) << quint8(0xF4) << false;
    QTest::newRow("overflow without carry") << quint8(0x50) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x50) << quint8(0x00) << quint8(0xA0) << quint8(0xF4) << false;
    QTest::newRow("carry zero overflow") << quint8(0x80) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x80) << quint8(0x00) << quint8(0x00) << quint8(0x77) << false;
    QTest::newRow("with page crossing") << quint8(0x10) << quint8(0x20) << quint8(0x20) << quint16(0x23F0) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << true;
    QTest::newRow("zero page pointer wraparound") << quint8(0x10) << quint8(0xFF) << quint8(0x00) << quint16(0x2345) << quint8(0x20) << quint8(0x00) << quint8(0x30) << quint8(0x34) << false;
}

void MOS6510TestArithmetic::testAdcIndirectIndexed()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, pointerAddress);
    QFETCH(quint8, y);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(y);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x71);  // ADC (Zero Page),Y
    m_memory.writeRAM(0x1001, pointerAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 pointerLow = static_cast<quint8>(targetAddress & 0x00FF);
    const quint8 pointerHigh = static_cast<quint8>((targetAddress >> 8) & 0x00FF);
    m_memory.writeRAM(pointerAddress, pointerLow);
    m_memory.writeRAM(static_cast<quint8>(pointerAddress + 1), pointerHigh);
    const quint16 effectiveAddress = static_cast<quint16>(targetAddress + y);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read target address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 4: Read target address high + add Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    if (pageCrossing)
    {
        // Cycle 5: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.yRegister(), y);
        QCOMPARE(m_cpu.status(), initialStatus);

        // Cycle 6: Read operand and ADC
        m_cpu.clock();
    }
    else
    {
        // Cycle 5: Read operand and ADC
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x30) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
    QTest::newRow("borrow") << quint8(0x30) << quint8(0x40) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("borrow with carry clear") << quint8(0x30) << quint8(0x10) << quint8(0x00) << quint8(0x1F) << quint8(0x35);
    QTest::newRow("zero") << quint8(0x40) << quint8(0x40) << quint8(0x01) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("overflow") << quint8(0x80) << quint8(0x01) << quint8(0x01) << quint8(0x7F) << quint8(0x75);
    QTest::newRow("overflow negative") << quint8(0x7F) << quint8(0xFF) << quint8(0x01) << quint8(0x80) << quint8(0xF4);
}
void MOS6510TestArithmetic::testSbcImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(static_cast<quint8>(0x74 | carryIn));
    m_memory.writeRAM(0x1000, 0xE9);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: SBC immediate
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcZeroPage_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x30) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
    QTest::newRow("borrow") << quint8(0x30) << quint8(0x40) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("borrow with carry clear") << quint8(0x30) << quint8(0x10) << quint8(0x00) << quint8(0x1F) << quint8(0x35);
    QTest::newRow("zero") << quint8(0x40) << quint8(0x40) << quint8(0x01) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("overflow") << quint8(0x80) << quint8(0x01) << quint8(0x01) << quint8(0x7F) << quint8(0x75);
    QTest::newRow("overflow negative") << quint8(0x7F) << quint8(0xFF) << quint8(0x01) << quint8(0x80) << quint8(0xF4);
}
void MOS6510TestArithmetic::testSbcZeroPage()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xE5);  // SBC Zero Page
    m_memory.writeRAM(0x1001, 0x20);  // Zero Page address
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    m_memory.writeRAM(0x0020, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 3: Read operand and SBC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcZeroPageX_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
    QTest::newRow("borrow") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint8(0x40) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("borrow with carry clear") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint8(0x10) << quint8(0x00) << quint8(0x1F) << quint8(0x35);
    QTest::newRow("zero") << quint8(0x40) << quint8(0x20) << quint8(0x05) << quint8(0x40) << quint8(0x01) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative") << quint8(0x10) << quint8(0x20) << quint8(0x05) << quint8(0x20) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("overflow") << quint8(0x80) << quint8(0x20) << quint8(0x05) << quint8(0x01) << quint8(0x01) << quint8(0x7F) << quint8(0x75);
    QTest::newRow("overflow negative") << quint8(0x7F) << quint8(0x20) << quint8(0x05) << quint8(0xFF) << quint8(0x01) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("zero page wraparound") << quint8(0x30) << quint8(0xF0) << quint8(0x20) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
}

void MOS6510TestArithmetic::testSbcZeroPageX()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xF5);  // SBC Zero Page,X
    m_memory.writeRAM(0x1001, baseAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 effectiveAddress = static_cast<quint8>(baseAddress + x);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read Zero Page base address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Add X to Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read operand and SBC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcAbsolute_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x30) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
    QTest::newRow("borrow") << quint8(0x30) << quint8(0x40) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("borrow with carry clear") << quint8(0x30) << quint8(0x10) << quint8(0x00) << quint8(0x1F) << quint8(0x35);
    QTest::newRow("zero") << quint8(0x40) << quint8(0x40) << quint8(0x01) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative") << quint8(0x10) << quint8(0x20) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("overflow") << quint8(0x80) << quint8(0x01) << quint8(0x01) << quint8(0x7F) << quint8(0x75);
    QTest::newRow("overflow negative") << quint8(0x7F) << quint8(0xFF) << quint8(0x01) << quint8(0x80) << quint8(0xF4);
}

void MOS6510TestArithmetic::testSbcAbsolute()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xED);  // SBC Absolute
    m_memory.writeRAM(0x1001, 0x45);  // Low byte
    m_memory.writeRAM(0x1002, 0x23);  // High byte
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    m_memory.writeRAM(0x2345, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 3: Read address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 4: Read operand and SBC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcAbsoluteX_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x30) << quint16(0x2340) << quint8(0x05) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << false;
    QTest::newRow("with page crossing") << quint8(0x30) << quint16(0x23F0) << quint8(0x20) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << true;
}
void MOS6510TestArithmetic::testSbcAbsoluteX()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xFD);  // SBC Absolute,X
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((baseAddress >> 8) & 0x00FF));
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    const quint16 effectiveAddress = static_cast<quint16>(baseAddress + x);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Read address high + add X
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    if (pageCrossing)
    {
        // Cycle 4: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.status(), initialStatus);

        // Cycle 5: Read operand and SBC
        m_cpu.clock();
    }
    else
    {
        // Cycle 4: Read operand and SBC
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcAbsoluteY_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x30) << quint16(0x2340) << quint8(0x05) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << false;
    QTest::newRow("with page crossing") << quint8(0x30) << quint16(0x23F0) << quint8(0x20) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << true;
}

void MOS6510TestArithmetic::testSbcAbsoluteY()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, y);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(y);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xF9);  // SBC Absolute,Y
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((baseAddress >> 8) & 0x00FF));
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    const quint16 effectiveAddress = static_cast<quint16>(baseAddress + y);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read address high + add Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    if (pageCrossing)
    {
        // Cycle 4: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.yRegister(), y);
        QCOMPARE(m_cpu.status(), initialStatus);

        // Cycle 5: Read operand and SBC
        m_cpu.clock();
    }
    else
    {
        // Cycle 4: Read operand and SBC
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcIndexedIndirect_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("pointerAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("positive") << quint8(0x30) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
    QTest::newRow("borrow") << quint8(0x30) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x40) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("borrow with carry clear") << quint8(0x30) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x10) << quint8(0x00) << quint8(0x1F) << quint8(0x35);
    QTest::newRow("zero") << quint8(0x40) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x40) << quint8(0x01) << quint8(0x00) << quint8(0x37);
    QTest::newRow("negative") << quint8(0x10) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x20) << quint8(0x01) << quint8(0xF0) << quint8(0xB4);
    QTest::newRow("overflow") << quint8(0x80) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x01) << quint8(0x01) << quint8(0x7F) << quint8(0x75);
    QTest::newRow("overflow negative") << quint8(0x7F) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0xFF) << quint8(0x01) << quint8(0x80) << quint8(0xF4);
    QTest::newRow("zero page pointer wraparound") << quint8(0x30) << quint8(0xFC) << quint8(0x04) << quint16(0x2345) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35);
}

void MOS6510TestArithmetic::testSbcIndexedIndirect()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, pointerAddress);
    QFETCH(quint8, x);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xE1);  // SBC (Zero Page,X)
    m_memory.writeRAM(0x1001, pointerAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 effectivePointer = static_cast<quint8>(pointerAddress + x);
    m_memory.writeRAM(effectivePointer, static_cast<quint8>(targetAddress & 0x00FF));
    m_memory.writeRAM(static_cast<quint8>(effectivePointer + 1), static_cast<quint8>((targetAddress >> 8) & 0x00FF));
    m_memory.writeRAM(targetAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Add X to Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read target address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 5: Read target address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 6: Read operand and SBC
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 7: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testSbcIndirectIndexed_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("pointerAddress");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << false;
    QTest::newRow("carry clear") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x10) << quint8(0x00) << quint8(0x1F) << quint8(0x35) << false;
    QTest::newRow("borrow") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x40) << quint8(0x01) << quint8(0xF0) << quint8(0xB4) << false;
    QTest::newRow("zero") << quint8(0x40) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x40) << quint8(0x01) << quint8(0x00) << quint8(0x37) << false;
    QTest::newRow("negative") << quint8(0x10) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x20) << quint8(0x01) << quint8(0xF0) << quint8(0xB4) << false;
    QTest::newRow("overflow") << quint8(0x80) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x01) << quint8(0x01) << quint8(0x7F) << quint8(0x75) << false;
    QTest::newRow("overflow negative") << quint8(0x7F) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0xFF) << quint8(0x01) << quint8(0x80) << quint8(0xF4) << false;
    QTest::newRow("with page crossing") << quint8(0x30) << quint8(0x20) << quint8(0x20) << quint16(0x23F0) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << true;
    QTest::newRow("zero page pointer wraparound") << quint8(0x30) << quint8(0xFF) << quint8(0x00) << quint16(0x2345) << quint8(0x10) << quint8(0x01) << quint8(0x20) << quint8(0x35) << false;
}

void MOS6510TestArithmetic::testSbcIndirectIndexed()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, pointerAddress);
    QFETCH(quint8, y);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(y);
    quint8 initialStatus = 0x74;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xF1);  // SBC (Zero Page),Y
    m_memory.writeRAM(0x1001, pointerAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 pointerLow = static_cast<quint8>(targetAddress & 0x00FF);
    const quint8 pointerHigh = static_cast<quint8>((targetAddress >> 8) & 0x00FF);
    m_memory.writeRAM(pointerAddress, pointerLow);
    m_memory.writeRAM(static_cast<quint8>(pointerAddress + 1), pointerHigh);
    const quint16 effectiveAddress = static_cast<quint16>(targetAddress + y);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read target address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 4: Read target address high + add Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    if (pageCrossing)
    {
        // Cycle 5: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.yRegister(), y);
        QCOMPARE(m_cpu.status(), initialStatus);

        // Cycle 6: Read operand and SBC
        m_cpu.clock();
    }
    else
    {
        // Cycle 5: Read operand and SBC
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), expectedAccumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}
void MOS6510TestArithmetic::testCmpImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xC9);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: CMP immediate
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpZeroPage_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}
void MOS6510TestArithmetic::testCmpZeroPage()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xC5);  // CMP Zero Page
    m_memory.writeRAM(0x1001, 0x20);  // Zero Page address
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    m_memory.writeRAM(0x0020, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 3: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpZeroPageX_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x20) << quint8(0x05) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x20) << quint8(0x05) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x20) << quint8(0x05) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0x20) << quint8(0x05) << quint8(0xFF) << quint8(0x77);
    QTest::newRow("zero page wraparound") << quint8(0x10) << quint8(0xF0) << quint8(0x20) << quint8(0x05) << quint8(0x75);
}
void MOS6510TestArithmetic::testCmpZeroPageX()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xD5);  // CMP Zero Page,X
    m_memory.writeRAM(0x1001, baseAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 effectiveAddress = static_cast<quint8>(baseAddress + x);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read Zero Page base address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Add X to Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpAbsolute_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}

void MOS6510TestArithmetic::testCmpAbsolute()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xCD);  // CMP Absolute
    m_memory.writeRAM(0x1001, 0x45);  // Low byte
    m_memory.writeRAM(0x1002, 0x23);  // High byte
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    m_memory.writeRAM(0x2345, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 3: Read address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);

    // Cycle 4: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpAbsoluteX_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x50) << quint16(0x2340) << quint8(0x05) << quint8(0x30) << quint8(0x75) << false;
    QTest::newRow("with page crossing") << quint8(0x50) << quint16(0x23F0) << quint8(0x20) << quint8(0x30) << quint8(0x75) << true;
}
void MOS6510TestArithmetic::testCmpAbsoluteX()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xDD);  // CMP Absolute,X
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((baseAddress >> 8) & 0x00FF));
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    const quint16 effectiveAddress = static_cast<quint16>(baseAddress + x);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Read address high + add X
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    if (pageCrossing)
    {
        // Cycle 4: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.xRegister(), x);
        QCOMPARE(m_cpu.status(), quint8(0x74));

        // Cycle 5: Read operand and compare
        m_cpu.clock();
    }
    else
    {
        // Cycle 4: Read operand and compare
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpAbsoluteY_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint16>("baseAddress");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("without page crossing") << quint8(0x50) << quint16(0x2340) << quint8(0x05) << quint8(0x30) << quint8(0x75) << false;
    QTest::newRow("with page crossing") << quint8(0x50) << quint16(0x23F0) << quint8(0x20) << quint8(0x30) << quint8(0x75) << true;
}
void MOS6510TestArithmetic::testCmpAbsoluteY()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint16, baseAddress);
    QFETCH(quint8, y);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(y);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xD9);  // CMP Absolute,Y
    m_memory.writeRAM(0x1001, static_cast<quint8>(baseAddress & 0x00FF));
    m_memory.writeRAM(0x1002, static_cast<quint8>((baseAddress >> 8) & 0x00FF));
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    const quint16 effectiveAddress = static_cast<quint16>(baseAddress + y);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read address high + add Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    if (pageCrossing)
    {
        // Cycle 4: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.yRegister(), y);
        QCOMPARE(m_cpu.status(), quint8(0x74));

        // Cycle 5: Read operand and compare
        m_cpu.clock();
    }
    else
    {
        // Cycle 4: Read operand and compare
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpIndexedIndirect_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("pointerAddress");
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0x20) << quint8(0x04) << quint16(0x2345) << quint8(0xFF) << quint8(0x77);
    QTest::newRow("zero page pointer wraparound") << quint8(0x50) << quint8(0xFC) << quint8(0x04) << quint16(0x2345) << quint8(0x30) << quint8(0x75);
}

void MOS6510TestArithmetic::testCmpIndexedIndirect()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, pointerAddress);
    QFETCH(quint8, x);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setXRegister(x);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xC1);  // CMP (Zero Page,X)
    m_memory.writeRAM(0x1001, pointerAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 effectivePointer = static_cast<quint8>(pointerAddress + x);
    m_memory.writeRAM(effectivePointer, static_cast<quint8>(targetAddress & 0x00FF));
    m_memory.writeRAM(static_cast<quint8>(effectivePointer + 1), static_cast<quint8>((targetAddress >> 8) & 0x00FF));
    m_memory.writeRAM(targetAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Add X to Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read target address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 5: Read target address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 6: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 7: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCmpIndirectIndexed_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("pointerAddress");
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint16>("targetAddress");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");
    QTest::addColumn<bool>("pageCrossing");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x30) << quint8(0x75) << false;
    QTest::newRow("equal") << quint8(0x40) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x40) << quint8(0x77) << false;
    QTest::newRow("less") << quint8(0x30) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x40) << quint8(0xF4) << false;
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0x01) << quint8(0xF4) << false;
    QTest::newRow("zero") << quint8(0xFF) << quint8(0x20) << quint8(0x05) << quint16(0x2340) << quint8(0xFF) << quint8(0x77) << false;
    QTest::newRow("with page crossing") << quint8(0x50) << quint8(0x20) << quint8(0x20) << quint16(0x23F0) << quint8(0x30) << quint8(0x75) << true;
    QTest::newRow("zero page pointer wraparound") << quint8(0x50) << quint8(0xFF) << quint8(0x00) << quint16(0x2345) << quint8(0x30) << quint8(0x75) << false;
}

void MOS6510TestArithmetic::testCmpIndirectIndexed()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, pointerAddress);
    QFETCH(quint8, y);
    QFETCH(quint16, targetAddress);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);
    QFETCH(bool, pageCrossing);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    m_cpu.setYRegister(y);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xD1);  // CMP (Zero Page),Y
    m_memory.writeRAM(0x1001, pointerAddress);
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    const quint8 pointerLow = static_cast<quint8>(targetAddress & 0x00FF);
    const quint8 pointerHigh = static_cast<quint8>((targetAddress >> 8) & 0x00FF);
    m_memory.writeRAM(pointerAddress, pointerLow);
    m_memory.writeRAM(static_cast<quint8>(pointerAddress + 1), pointerHigh);
    const quint16 effectiveAddress = static_cast<quint16>(targetAddress + y);
    m_memory.writeRAM(effectiveAddress, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read Zero Page pointer
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read target address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 4: Read target address high + add Y
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);

    if (pageCrossing)
    {
        // Cycle 5: Page-crossing dummy read
        m_cpu.clock();
        QCOMPARE(m_cpu.accumulator(), accumulator);
        QCOMPARE(m_cpu.yRegister(), y);
        QCOMPARE(m_cpu.status(), quint8(0x74));

        // Cycle 6: Read operand and compare
        m_cpu.clock();
    }
    else
    {
        // Cycle 5: Read operand and compare
        m_cpu.clock();
    }

    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Next cycle: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.accumulator(), accumulator);
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCpxImmediate_data()
{
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}
void MOS6510TestArithmetic::testCpxImmediate()
{
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setXRegister(x);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xE0);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: CPX immediate
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCpxZeroPage_data()
{
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}

void MOS6510TestArithmetic::testCpxZeroPage()
{
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setXRegister(x);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xE4);  // CPX Zero Page
    m_memory.writeRAM(0x1001, 0x20);  // Zero Page address
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    m_memory.writeRAM(0x0020, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCpxAbsolute_data()
{
    QTest::addColumn<quint8>("x");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}
void MOS6510TestArithmetic::testCpxAbsolute()
{
    QFETCH(quint8, x);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setXRegister(x);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xEC);  // CPX Absolute
    m_memory.writeRAM(0x1001, 0x45);  // Low byte
    m_memory.writeRAM(0x1002, 0x23);  // High byte
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    m_memory.writeRAM(0x2345, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 3: Read address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.xRegister(), x);

    // Cycle 4: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.xRegister(), x);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCpyImmediate_data()
{
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}
void MOS6510TestArithmetic::testCpyImmediate()
{
    QFETCH(quint8, y);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(y);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xC0);
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: CPY immediate
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCpyZeroPage_data()
{
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}

void MOS6510TestArithmetic::testCpyZeroPage()
{
    QFETCH(quint8, y);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(y);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xC4);  // CPY Zero Page
    m_memory.writeRAM(0x1001, 0x20);  // Zero Page address
    m_memory.writeRAM(0x1002, 0xEA);  // NOP
    m_memory.writeRAM(0x0020, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Zero Page address
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 4: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestArithmetic::testCpyAbsolute_data()
{
    QTest::addColumn<quint8>("y");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("greater") << quint8(0x50) << quint8(0x30) << quint8(0x75);
    QTest::newRow("equal") << quint8(0x40) << quint8(0x40) << quint8(0x77);
    QTest::newRow("less") << quint8(0x30) << quint8(0x40) << quint8(0xF4);
    QTest::newRow("negative result") << quint8(0x00) << quint8(0x01) << quint8(0xF4);
    QTest::newRow("zero") << quint8(0xFF) << quint8(0xFF) << quint8(0x77);
}
void MOS6510TestArithmetic::testCpyAbsolute()
{
    QFETCH(quint8, y);
    QFETCH(quint8, operand);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setYRegister(y);
    m_cpu.setStatus(0x74);
    m_memory.writeRAM(0x1000, 0xCC);  // CPX Absolute
    m_memory.writeRAM(0x1001, 0x45);  // Low byte
    m_memory.writeRAM(0x1002, 0x23);  // High byte
    m_memory.writeRAM(0x1003, 0xEA);  // NOP
    m_memory.writeRAM(0x2345, operand);
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 2: Read address low
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 3: Read address high
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.yRegister(), y);

    // Cycle 4: Read operand and compare
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1003));
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 5: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1004));
    QCOMPARE(m_cpu.yRegister(), y);
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
