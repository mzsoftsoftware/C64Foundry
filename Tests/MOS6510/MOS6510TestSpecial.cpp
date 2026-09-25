#include "MOS6510TestSpecial.h"

#include <QTest>

MOS6510TestSpecial::MOS6510TestSpecial()
{
}
MOS6510TestSpecial::~MOS6510TestSpecial()
{
}

void MOS6510TestSpecial::testNOP()
{
    setupCpu();
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setStatus(0x7D);
    m_memory.writeRAM(0x1000, 0xEA);
    m_memory.writeRAM(0x1001, 0xEA);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 2: NOP execution
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.status(), quint8(0x7D));

    // Cycle 3: Next Opcode Fetch
    m_cpu.clock();
    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));
    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.status(), quint8(0x7D));
}

void MOS6510TestSpecial::testFlagInstructions_data()
{
    QTest::addColumn<quint8>("opcode");
    QTest::addColumn<quint8>("initialStatus");
    QTest::addColumn<quint8>("expectedStatus");

    // CLC: clear Carry
    // Start with C set.
    QTest::newRow("CLC")
        << quint8(0x18)
        << quint8(0xFF)
        << quint8(0xFE);

    // SEC: set Carry
    // Start with C clear.
    QTest::newRow("SEC")
        << quint8(0x38)
        << quint8(0xFE)
        << quint8(0xFF);

    // CLI: clear Interrupt Disable
    // Start with I set.
    QTest::newRow("CLI")
        << quint8(0x58)
        << quint8(0xFF)
        << quint8(0xFB);

    // SEI: set Interrupt Disable
    // Start with I clear.
    QTest::newRow("SEI")
        << quint8(0x78)
        << quint8(0xFB)
        << quint8(0xFF);

    // CLV: clear Overflow
    // Start with V set.
    QTest::newRow("CLV")
        << quint8(0xB8)
        << quint8(0xFF)
        << quint8(0xBF);
}

void MOS6510TestSpecial::testFlagInstructions()
{
    QFETCH(quint8, opcode);
    QFETCH(quint8, initialStatus);
    QFETCH(quint8, expectedStatus);

    setupCpu();

    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x22);
    m_cpu.setYRegister(0x33);
    m_cpu.setStackPointer(0x44);
    m_cpu.setStatus(initialStatus);
    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, opcode);
    m_memory.writeRAM(0x1001, 0xEA);

    // Cycle 1: Opcode Fetch
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));

    // Flag must not have changed during opcode fetch.
    QCOMPARE(m_cpu.status(), initialStatus);

    // Cycle 2: Execute flag instruction
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1001));

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));

    QCOMPARE(m_cpu.status(), expectedStatus);

    // Cycle 3: Fetch next opcode.
    // This proves that the flag instruction takes exactly two cycles.
    m_cpu.clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x1002));

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));
    QCOMPARE(m_cpu.xRegister(), quint8(0x22));
    QCOMPARE(m_cpu.yRegister(), quint8(0x33));
    QCOMPARE(m_cpu.stackPointer(), quint8(0x44));

    QCOMPARE(m_cpu.status(), expectedStatus);
}
