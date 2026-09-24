#include "MOS6510TestDecimal.h"

#include <QTest>

namespace
{
struct DecimalReferenceResult
{
    quint8 accumulator;
    bool carry;
    bool negative;
    bool overflow;
    bool zero;
};
DecimalReferenceResult referenceAdcDecimal(
    quint8 accumulator,
    quint8 operand,
    bool carryIn)
{
    DecimalReferenceResult result{};

    unsigned temp =
        (accumulator & 0x0F) +
        (operand & 0x0F) +
        (carryIn ? 1U : 0U);

    if (temp > 9)
        temp += 6;

    if (temp <= 0x0F)
    {
        temp =
            (temp & 0x0F) +
            (accumulator & 0xF0) +
            (operand & 0xF0);
    }
    else
    {
        temp =
            (temp & 0x0F) +
            (accumulator & 0xF0) +
            (operand & 0xF0) +
            0x10;
    }

    //
    // NMOS ADC:
    // Z comes from the unadjusted binary result.
    //
    const quint8 binaryResult =
        static_cast<quint8>(
            accumulator +
            operand +
            (carryIn ? 1U : 0U));

    result.zero = binaryResult == 0;

    //
    // N and V come from the intermediate result,
    // before the high-digit decimal correction.
    //
    result.negative = (temp & 0x80) != 0;

    result.overflow =
        (((accumulator ^ temp) & 0x80) != 0) &&
        (((accumulator ^ operand) & 0x80) == 0);

    //
    // High-digit decimal correction.
    //
    if ((temp & 0x1F0) > 0x90)
        temp += 0x60;

    result.carry = (temp & 0xFF0) > 0xF0;

    result.accumulator =
        static_cast<quint8>(temp);

    return result;
}

DecimalReferenceResult referenceSbcDecimal(
    quint8 accumulator,
    quint8 operand,
    bool carryIn)
{
    DecimalReferenceResult result{};

    //
    // Binary subtraction determines all NMOS SBC flags.
    //
    const quint16 binaryResult =
        static_cast<quint16>(accumulator) +
        static_cast<quint8>(~operand) +
        (carryIn ? 1U : 0U);

    const quint8 binaryResult8 =
        static_cast<quint8>(binaryResult);

    result.negative = (binaryResult8 & 0x80) != 0;
    result.zero = binaryResult8 == 0;
    result.carry = binaryResult > 0xFF;

    result.overflow =
        (((accumulator ^ binaryResult8) & 0x80) != 0) &&
        (((accumulator ^ operand) & 0x80) != 0);

    //
    // NMOS decimal correction.
    //
    unsigned temp =
        (accumulator & 0x0F) -
        (operand & 0x0F) -
        (carryIn ? 0U : 1U);

    if (temp & 0x10)
    {
        temp =
            ((temp - 6U) & 0x0F) |
            ((accumulator & 0xF0) -
             (operand & 0xF0) -
             0x10U);
    }
    else
    {
        temp =
            (temp & 0x0F) |
            ((accumulator & 0xF0) -
             (operand & 0xF0));
    }

    if (temp & 0x100)
        temp -= 0x60;

    result.accumulator =
        static_cast<quint8>(temp);

    return result;
}
}

MOS6510TestDecimal::MOS6510TestDecimal()
{
}
MOS6510TestDecimal::~MOS6510TestDecimal()
{
}

void MOS6510TestDecimal::initializeRegisters()
{
    m_cpu.setAccumulator(0x55);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0xFF);

    // N V - B D I Z C
    // 0 0 1 1 1 1 0 C
    m_cpu.setStatus(0x3C);
}

// --------------------------------------------------------------------------------------------
void MOS6510TestDecimal::testSed_data()
{
    QTest::addColumn<quint8>("initialStatus");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("decimal clear") << quint8(0x35) << quint8(0x3D);
    QTest::newRow("decimal already set") << quint8(0x3D) << quint8(0x3D);
}

void MOS6510TestDecimal::testSed()
{
    QFETCH(quint8, initialStatus);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xF8); // SED
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: SED
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestDecimal::testCld_data()
{
    QTest::addColumn<quint8>("initialStatus");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("decimal set") << quint8(0x3D) << quint8(0x35);
    QTest::newRow("decimal already clear") << quint8(0x35) << quint8(0x35);
}

void MOS6510TestDecimal::testCld()
{
    QFETCH(quint8, initialStatus);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xD8); // CLD
    m_memory.writeRAM(0x1001, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // Cycle 1: Opcode fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: CLD
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.status(), expectedStatus);
}
// --------------------------------------------------------------------------------------------
void MOS6510TestDecimal::testDecimalModeSwitch()
{
    setupCpu();
    initializeRegisters();

    m_cpu.setAccumulator(0x08);
    // Start with Decimal Mode disabled.
    m_cpu.setStatus(0x34);
    m_memory.writeRAM(0x1000, 0xD8); // CLD
    m_memory.writeRAM(0x1001, 0x69); // ADC #imm
    m_memory.writeRAM(0x1002, 0x01);
    m_memory.writeRAM(0x1003, 0xF8); // SED
    m_memory.writeRAM(0x1004, 0x69); // ADC #imm
    m_memory.writeRAM(0x1005, 0x01);
    m_memory.writeRAM(0x1006, 0xEA); // NOP
    m_cpu.setProgramCounter(0x1000);

    // CLD
    m_cpu.clock();
    m_cpu.clock();
    QCOMPARE(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Decimal), quint8(0));

    // ADC #$01 in Binary Mode: $08 + $01 = $09
    m_cpu.clock();
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), quint8(0x09));

    // SED
    m_cpu.clock();
    m_cpu.clock();
    QCOMPARE(m_cpu.status() & static_cast<quint8>(MOS6510StatusFlag::Decimal), static_cast<quint8>(MOS6510StatusFlag::Decimal));

    // ADC #$01 in Decimal Mode: $09 + $01 = $10
    m_cpu.clock();
    m_cpu.clock();
    QCOMPARE(m_cpu.accumulator(), quint8(0x10));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestDecimal::testAdcImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("00 + 00")
        << quint8(0x00) << quint8(0x00) << quint8(0)
        << quint8(0x00) << quint8(0x3E);

    QTest::newRow("01 + 01")
        << quint8(0x01) << quint8(0x01) << quint8(0)
        << quint8(0x02) << quint8(0x3C);

    QTest::newRow("09 + 01")
        << quint8(0x09) << quint8(0x01) << quint8(0)
        << quint8(0x10) << quint8(0x3C);

    QTest::newRow("10 + 20")
        << quint8(0x10) << quint8(0x20) << quint8(0)
        << quint8(0x30) << quint8(0x3C);

    QTest::newRow("10 + 20 + carry")
        << quint8(0x10) << quint8(0x20) << quint8(1)
        << quint8(0x31) << quint8(0x3C);

    QTest::newRow("49 + 50")
        << quint8(0x49) << quint8(0x50) << quint8(0)
        << quint8(0x99) << quint8(0xFC);

    QTest::newRow("50 + 50")
        << quint8(0x50) << quint8(0x50) << quint8(0)
        << quint8(0x00) << quint8(0xFD);

    QTest::newRow("99 + 01")
        << quint8(0x99) << quint8(0x01) << quint8(0)
        << quint8(0x00) << quint8(0xBD);

    QTest::newRow("99 + 00 + carry")
        << quint8(0x99) << quint8(0x00) << quint8(1)
        << quint8(0x00) << quint8(0xBD);

    QTest::newRow("45 + 55")
        << quint8(0x45) << quint8(0x55) << quint8(0)
        << quint8(0x00) << quint8(0xFD);

    QTest::newRow("24 + 56")
        << quint8(0x24) << quint8(0x56) << quint8(0)
        << quint8(0x80) << quint8(0xFC);

    QTest::newRow("93 + 82")
        << quint8(0x93) << quint8(0x82) << quint8(0)
        << quint8(0x75) << quint8(0x7D);

    QTest::newRow("89 + 76")
        << quint8(0x89) << quint8(0x76) << quint8(0)
        << quint8(0x65) << quint8(0x3D);

    QTest::newRow("89 + 76 + carry")
        << quint8(0x89) << quint8(0x76) << quint8(1)
        << quint8(0x66) << quint8(0x3F);

    QTest::newRow("80 + F0")
        << quint8(0x80) << quint8(0xF0) << quint8(0)
        << quint8(0xD0) << quint8(0x7D);

    QTest::newRow("80 + FA")
        << quint8(0x80) << quint8(0xFA) << quint8(0)
        << quint8(0xE0) << quint8(0xBD);

    QTest::newRow("2F + 4F")
        << quint8(0x2F) << quint8(0x4F) << quint8(0)
        << quint8(0x74) << quint8(0x3C);

    QTest::newRow("6F + 00 + carry")
        << quint8(0x6F) << quint8(0x00) << quint8(1)
        << quint8(0x76) << quint8(0x3C);
}
void MOS6510TestDecimal::testAdcImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    quint8 initialStatus = 0x3C;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0x69); // ADC #imm
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA); // NOP
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
void MOS6510TestDecimal::testSbcImmediate_data()
{
    QTest::addColumn<quint8>("accumulator");
    QTest::addColumn<quint8>("operand");
    QTest::addColumn<quint8>("carryIn");
    QTest::addColumn<quint8>("expectedAccumulator");
    QTest::addColumn<quint8>("expectedStatus");

    QTest::newRow("00 - 00 borrow") << quint8(0x00) << quint8(0x00) << quint8(0) << quint8(0x99) << quint8(0xBC);
    QTest::newRow("00 - 00") << quint8(0x00) << quint8(0x00) << quint8(1) << quint8(0x00) << quint8(0x3F);
    QTest::newRow("00 - 01") << quint8(0x00) << quint8(0x01) << quint8(1) << quint8(0x99) << quint8(0xBC);
    QTest::newRow("0A - 00") << quint8(0x0A) << quint8(0x00) << quint8(1) << quint8(0x0A) << quint8(0x3D);
    QTest::newRow("0B - 00 borrow") << quint8(0x0B) << quint8(0x00) << quint8(0) << quint8(0x0A) << quint8(0x3D);
    QTest::newRow("9A - 00") << quint8(0x9A) << quint8(0x00) << quint8(1) << quint8(0x9A) << quint8(0xBD);
    QTest::newRow("9B - 00 borrow") << quint8(0x9B) << quint8(0x00) << quint8(0) << quint8(0x9A) << quint8(0xBD);
    QTest::newRow("10 - 01") << quint8(0x10) << quint8(0x01) << quint8(1) << quint8(0x09) << quint8(0x3D);
    QTest::newRow("10 - 01 borrow") << quint8(0x10) << quint8(0x01) << quint8(0) << quint8(0x08) << quint8(0x3D);
    QTest::newRow("20 - 10") << quint8(0x20) << quint8(0x10) << quint8(1) << quint8(0x10) << quint8(0x3D);
    QTest::newRow("20 - 10 borrow") << quint8(0x20) << quint8(0x10) << quint8(0) << quint8(0x09) << quint8(0x3D);
    QTest::newRow("50 - 50") << quint8(0x50) << quint8(0x50) << quint8(1) << quint8(0x00) << quint8(0x3F);
    QTest::newRow("80 - 01") << quint8(0x80) << quint8(0x01) << quint8(1) << quint8(0x79) << quint8(0x7D);
    QTest::newRow("7F - FF") << quint8(0x7F) << quint8(0xFF) << quint8(1) << quint8(0x20) << quint8(0xFC);
    QTest::newRow("00 - 01 borrow") << quint8(0x00) << quint8(0x01) << quint8(0) << quint8(0x98) << quint8(0xBC);
    QTest::newRow("10 - 20") << quint8(0x10) << quint8(0x20) << quint8(1) << quint8(0x90) << quint8(0xBC);
}
void MOS6510TestDecimal::testSbcImmediate()
{
    QFETCH(quint8, accumulator);
    QFETCH(quint8, operand);
    QFETCH(quint8, carryIn);
    QFETCH(quint8, expectedAccumulator);
    QFETCH(quint8, expectedStatus);

    setupCpu();
    initializeRegisters();
    m_cpu.setAccumulator(accumulator);
    quint8 initialStatus = 0x3C;
    if (carryIn != 0)
        initialStatus |= static_cast<quint8>(MOS6510StatusFlag::Carry);
    m_cpu.setStatus(initialStatus);
    m_memory.writeRAM(0x1000, 0xE9); // SBC #imm
    m_memory.writeRAM(0x1001, operand);
    m_memory.writeRAM(0x1002, 0xEA); // NOP
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
void MOS6510TestDecimal::testAdcDecimalExhaustive()
{
    quint32 testCount = 0;
    for (int accumulator = 0; accumulator <= 0xFF; ++accumulator)
    {
        for (int operand = 0; operand <= 0xFF; ++operand)
        {
            for (int carryIn = 0; carryIn <= 1; ++carryIn)
            {
                ++testCount;
                const auto expected = referenceAdcDecimal(static_cast<quint8>(accumulator), static_cast<quint8>(operand), carryIn != 0);
                setupCpu();
                m_cpu.setAccumulator(static_cast<quint8>(accumulator));
                quint8 status = 0x3C;
                if (carryIn != 0)
                    status |= static_cast<quint8>(MOS6510StatusFlag::Carry);
                m_cpu.setStatus(status);
                m_memory.writeRAM(0x1000, 0x69); // ADC #imm
                m_memory.writeRAM(0x1001, static_cast<quint8>(operand));
                m_cpu.setProgramCounter(0x1000);
                m_cpu.clock(); // Fetch
                m_cpu.clock(); // Execute
                const quint8 actualStatus = m_cpu.status();
                const bool actualCarry = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0;
                const bool actualNegative = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Negative)) != 0;
                const bool actualOverflow = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Overflow)) != 0;
                const bool actualZero = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Zero)) != 0;
                if (m_cpu.accumulator() != expected.accumulator ||
                    actualCarry != expected.carry ||
                    actualNegative != expected.negative ||
                    actualOverflow != expected.overflow ||
                    actualZero != expected.zero)
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "ADC decimal mismatch: "
                                "A=$%1 operand=$%2 C=%3 | "
                                "actual A=$%4 N=%5 V=%6 Z=%7 C=%8 | "
                                "expected A=$%9 N=%10 V=%11 Z=%12 C=%13")
                                .arg(accumulator, 2, 16, QLatin1Char('0'))
                                .arg(operand, 2, 16, QLatin1Char('0'))
                                .arg(carryIn)
                                .arg(m_cpu.accumulator(), 2, 16, QLatin1Char('0'))
                                .arg(actualNegative)
                                .arg(actualOverflow)
                                .arg(actualZero)
                                .arg(actualCarry)
                                .arg(expected.accumulator, 2, 16, QLatin1Char('0'))
                                .arg(expected.negative)
                                .arg(expected.overflow)
                                .arg(expected.zero)
                                .arg(expected.carry)));
                }
            }
        }
    }
    QCOMPARE(testCount, quint32(256 * 256 * 2));
}
// --------------------------------------------------------------------------------------------
void MOS6510TestDecimal::testSbcDecimalExhaustive()
{
    quint32 testCount = 0;
    for (int accumulator = 0; accumulator <= 0xFF; ++accumulator)
    {
        for (int operand = 0; operand <= 0xFF; ++operand)
        {
            for (int carryIn = 0; carryIn <= 1; ++carryIn)
            {
                ++testCount;
                const auto expected = referenceSbcDecimal(static_cast<quint8>(accumulator), static_cast<quint8>(operand), carryIn != 0);
                setupCpu();
                m_cpu.setAccumulator(static_cast<quint8>(accumulator));
                quint8 status = 0x3C;
                if (carryIn != 0)
                    status |= static_cast<quint8>(MOS6510StatusFlag::Carry);
                m_cpu.setStatus(status);
                m_memory.writeRAM(0x1000, 0xE9); // SBC #imm
                m_memory.writeRAM(0x1001, static_cast<quint8>(operand));
                m_cpu.setProgramCounter(0x1000);
                m_cpu.clock(); // Fetch
                m_cpu.clock(); // Execute
                const quint8 actualStatus = m_cpu.status();
                const bool actualCarry = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0;
                const bool actualNegative = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Negative)) != 0;
                const bool actualOverflow = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Overflow)) != 0;
                const bool actualZero = (actualStatus & static_cast<quint8>(MOS6510StatusFlag::Zero)) != 0;
                if (m_cpu.accumulator() != expected.accumulator ||
                    actualCarry != expected.carry ||
                    actualNegative != expected.negative ||
                    actualOverflow != expected.overflow ||
                    actualZero != expected.zero)
                {
                    QFAIL(
                        qPrintable(
                            QStringLiteral(
                                "SBC decimal mismatch: "
                                "A=$%1 operand=$%2 C=%3 | "
                                "actual A=$%4 N=%5 V=%6 Z=%7 C=%8 | "
                                "expected A=$%9 N=%10 V=%11 Z=%12 C=%13")
                                .arg(accumulator, 2, 16, QLatin1Char('0'))
                                .arg(operand, 2, 16, QLatin1Char('0'))
                                .arg(carryIn)
                                .arg(m_cpu.accumulator(), 2, 16, QLatin1Char('0'))
                                .arg(actualNegative)
                                .arg(actualOverflow)
                                .arg(actualZero)
                                .arg(actualCarry)
                                .arg(expected.accumulator, 2, 16, QLatin1Char('0'))
                                .arg(expected.negative)
                                .arg(expected.overflow)
                                .arg(expected.zero)
                                .arg(expected.carry)));
                }
            }
        }
    }
    QCOMPARE(testCount, quint32(256 * 256 * 2));
}
