#include "MOS6510TestBusCycles.h"

#include <QtTest>


MOS6510TestBusCycles::MOS6510TestBusCycles()
{
}
MOS6510TestBusCycles::~MOS6510TestBusCycles()
{
}

void MOS6510TestBusCycles::init()
{
    setupCpu();
    m_cpu.setProgramCounter(0x1000);
}
void MOS6510TestBusCycles::clock()
{
    m_bus.clock();
    m_cpu.clock();
}

void MOS6510TestBusCycles::verifyRead(const quint16 address, const quint8 value)
{
    QCOMPARE(m_bus.accessCount(), quint8(1));
    QCOMPARE(m_bus.lastAccessType(), C64Bus::AccessType::Read);
    QCOMPARE(m_bus.lastAccessAddress(), address);
    QCOMPARE(m_bus.lastAccessValue(), value);
}
void MOS6510TestBusCycles::verifyWrite(const quint16 address, const quint8 value)
{
    QCOMPARE(m_bus.accessCount(), quint8(1));
    QCOMPARE(m_bus.lastAccessType(), C64Bus::AccessType::Write);
    QCOMPARE(m_bus.lastAccessAddress(), address);
    QCOMPARE(m_bus.lastAccessValue(), value);
}
void MOS6510TestBusCycles::verifyNoAccess()
{
    QCOMPARE(m_bus.accessCount(), quint8(0));
    QCOMPARE(m_bus.lastAccessType(), C64Bus::AccessType::None);
}

void MOS6510TestBusCycles::testAdcImmediate()
{
    m_memory.writeRAM(0x1000, 0x69);    // ADC #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);    // NOP

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x69);

    clock();                            // C2: Read operand
    verifyRead(0x1001, 0x42);

    clock();                            // C1: Fetch next opcode
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcZeroPage()
{
    m_memory.writeRAM(0x1000, 0x65);    // ADC $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1
    verifyRead(0x1000, 0x65);

    clock();                            // C2
    verifyRead(0x1001, 0x20);

    clock();                            // C3
    verifyRead(0x0020, 0x42);

    clock();                            // Next opcode
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x75);    // ADC $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0x75);

    clock();                            // C2
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read
    verifyRead(0x0020, 0x11);

    clock();                            // C4
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x75);    // ADC $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x75);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read before indexing
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcAbsolute()
{
    m_memory.writeRAM(0x1000, 0x6D);    // ADC $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0x6D);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAdcAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x7D);    // ADC $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x7D);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAdcAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x7D);    // ADC $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x2403, 0x42);    // Effective address
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x7D);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Corrected address
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAdcAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x79);    // ADC $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x79);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAdcAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x79);    // ADC $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x79);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAdcIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0x61);    // ADC ($20,X)
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x11);    // Dummy read
    m_memory.writeRAM(0x0025, 0x45);    // Pointer low
    m_memory.writeRAM(0x0026, 0x23);    // Pointer high
    m_memory.writeRAM(0x2345, 0x42);

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x61);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x45);

    clock();
    verifyRead(0x0026, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0x61);    // ADC ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);

    m_memory.writeRAM(0x00FC, 0x11);    // Dummy read
    m_memory.writeRAM(0x00FF, 0x45);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high after ZP wrap
    m_memory.writeRAM(0x2345, 0x42);

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0x61);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();
    verifyRead(0x00FC, 0x11);

    clock();
    verifyRead(0x00FF, 0x45);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x71);    // ADC ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x40);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2345, 0x42);

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x71);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x40);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x71);    // ADC ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0x23);

    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x71);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();                            // Corrected address
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAdcIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0x71);    // ADC ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);

    m_memory.writeRAM(0x00FF, 0x40);
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP
    m_memory.writeRAM(0x2345, 0x42);

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x71);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x40);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAndImmediate()
{
    m_memory.writeRAM(0x1000, 0x29);
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x29);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndZeroPage()
{
    m_memory.writeRAM(0x1000, 0x25);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x25);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x35);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x35);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x35);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x35);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndAbsolute()
{
    m_memory.writeRAM(0x1000, 0x2D);
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0x2D);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testAndAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x3D);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x3D);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testAndAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x3D);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x3D);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testAndAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x39);
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x39);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testAndAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x39);
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x39);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testAndIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0x21);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x45);
    m_memory.writeRAM(0x0026, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x21);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x45);

    clock();
    verifyRead(0x0026, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0x21);
    m_memory.writeRAM(0x1001, 0xFC);
    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);
    m_memory.writeRAM(0x0000, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0x21);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();
    verifyRead(0x00FC, 0x11);

    clock();
    verifyRead(0x00FF, 0x45);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x31);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x40);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x31);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x40);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x31);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x31);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testAndIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0x31);
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x00FF, 0x40);
    m_memory.writeRAM(0x0000, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x31);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x40);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAslAccumulator()
{
    m_memory.writeRAM(0x1000, 0x0A);    // ASL A
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1
    verifyRead(0x1000, 0x0A);

    clock();                            // C2: Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Next opcode fetch
    verifyRead(0x1001, 0xEA);
}
void MOS6510TestBusCycles::testAslZeroPage()
{
    m_memory.writeRAM(0x1000, 0x06);    // ASL $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x21);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1
    verifyRead(0x1000, 0x06);

    clock();                            // C2
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Read old
    verifyRead(0x0020, 0x21);

    clock();                            // C4: Dummy write old
    verifyWrite(0x0020, 0x21);

    clock();                            // C5: Write new
    verifyWrite(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAslZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x16);    // ASL $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x21);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0x16);

    clock();                            // C2
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Read old
    verifyRead(0x0025, 0x21);

    clock();                            // C5: Dummy write old
    verifyWrite(0x0025, 0x21);

    clock();                            // C6: Write new
    verifyWrite(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAslZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x16);    // ASL $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x21);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x16);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x21);

    clock();
    verifyWrite(0x0003, 0x21);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testAslAbsolute()
{
    m_memory.writeRAM(0x1000, 0x0E);    // ASL $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x21);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0x0E);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x21);

    clock();
    verifyWrite(0x2345, 0x21);

    clock();
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAslAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x1E);    // ASL $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2345, 0x21);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0x1E);

    clock();                            // C2
    verifyRead(0x1001, 0x40);

    clock();                            // C3
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Indexed dummy read
    verifyRead(0x2345, 0x21);

    clock();                            // C5: Read old
    verifyRead(0x2345, 0x21);

    clock();                            // C6: Write old
    verifyWrite(0x2345, 0x21);

    clock();                            // C7: Write new
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testAslAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x1E);    // ASL $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x2403, 0x21);    // Actual operand
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0x1E);

    clock();                            // C2
    verifyRead(0x1001, 0xFE);

    clock();                            // C3
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Read old
    verifyRead(0x2403, 0x21);

    clock();                            // C6: Write old
    verifyWrite(0x2403, 0x21);

    clock();                            // C7: Write new
    verifyWrite(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testBccNotTaken()
{
    m_memory.writeRAM(0x1000, 0x90);    // BCC +$05
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x01);              // C = 1 -> branch not taken

    clock();
    verifyRead(0x1000, 0x90);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBccTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x90);    // BCC +$05
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x00);              // C = 0 -> branch taken

    clock();
    verifyRead(0x1000, 0x90);

    clock();
    verifyRead(0x1001, 0x05);

    clock();                            // Dummy read at old PC
    verifyRead(0x1002, 0x11);

    clock();                            // Fetch branch target
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBccTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0x90);    // BCC +$05
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);    // Wrong-page read
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x10FC, 0x90);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();                            // Low byte adjusted, old high byte
    verifyRead(0x1003, 0x22);

    clock();                            // Fetch branch target
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBcsNotTaken()
{
    m_memory.writeRAM(0x1000, 0xB0);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x00);              // C = 0

    clock();
    verifyRead(0x1000, 0xB0);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBcsTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xB0);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x01);              // C = 1

    clock();
    verifyRead(0x1000, 0xB0);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBcsTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0xB0);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x01);

    clock();
    verifyRead(0x10FC, 0xB0);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBeqNotTaken()
{
    m_memory.writeRAM(0x1000, 0xF0);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x00);              // Z = 0

    clock();
    verifyRead(0x1000, 0xF0);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBeqTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xF0);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x02);              // Z = 1

    clock();
    verifyRead(0x1000, 0xF0);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBeqTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0xF0);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x02);

    clock();
    verifyRead(0x10FC, 0xF0);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBmiNotTaken()
{
    m_memory.writeRAM(0x1000, 0x30);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x00);              // N = 0

    clock();
    verifyRead(0x1000, 0x30);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBmiTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x30);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x80);              // N = 1

    clock();
    verifyRead(0x1000, 0x30);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBmiTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0x30);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x80);

    clock();
    verifyRead(0x10FC, 0x30);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBneNotTaken()
{
    m_memory.writeRAM(0x1000, 0xD0);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x02);              // Z = 1

    clock();
    verifyRead(0x1000, 0xD0);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBneTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xD0);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x00);              // Z = 0

    clock();
    verifyRead(0x1000, 0xD0);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBneTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0xD0);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x10FC, 0xD0);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBplNotTaken()
{
    m_memory.writeRAM(0x1000, 0x10);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x80);              // N = 1

    clock();
    verifyRead(0x1000, 0x10);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBplTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x10);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x00);              // N = 0

    clock();
    verifyRead(0x1000, 0x10);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBplTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0x10);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x10FC, 0x10);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBvcNotTaken()
{
    m_memory.writeRAM(0x1000, 0x50);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x40);              // V = 1

    clock();
    verifyRead(0x1000, 0x50);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBvcTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x50);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x00);              // V = 0

    clock();
    verifyRead(0x1000, 0x50);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBvcTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0x50);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x10FC, 0x50);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBvsNotTaken()
{
    m_memory.writeRAM(0x1000, 0x70);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x00);              // V = 0

    clock();
    verifyRead(0x1000, 0x70);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBvsTakenWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x70);
    m_memory.writeRAM(0x1001, 0x05);
    m_memory.writeRAM(0x1002, 0x11);
    m_memory.writeRAM(0x1007, 0xEA);

    m_cpu.setStatus(0x40);              // V = 1

    clock();
    verifyRead(0x1000, 0x70);

    clock();
    verifyRead(0x1001, 0x05);

    clock();
    verifyRead(0x1002, 0x11);

    clock();
    verifyRead(0x1007, 0xEA);
}

void MOS6510TestBusCycles::testBvsTakenWithPageCrossing()
{
    m_cpu.setProgramCounter(0x10FC);

    m_memory.writeRAM(0x10FC, 0x70);
    m_memory.writeRAM(0x10FD, 0x05);
    m_memory.writeRAM(0x10FE, 0x11);
    m_memory.writeRAM(0x1003, 0x22);
    m_memory.writeRAM(0x1103, 0xEA);

    m_cpu.setStatus(0x40);

    clock();
    verifyRead(0x10FC, 0x70);

    clock();
    verifyRead(0x10FD, 0x05);

    clock();
    verifyRead(0x10FE, 0x11);

    clock();
    verifyRead(0x1003, 0x22);

    clock();
    verifyRead(0x1103, 0xEA);
}
void MOS6510TestBusCycles::testBitZeroPage()
{
    m_memory.writeRAM(0x1000, 0x24);
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x24);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testBitAbsolute()
{
    m_memory.writeRAM(0x1000, 0x2C);
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0x2C);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testBrk()
{
    m_memory.writeRAM(0x1000, 0x00);    // BRK
    m_memory.writeRAM(0x1001, 0x42);    // Padding byte

    m_memory.writeRAM(0xFFFE, 0x45);
    m_memory.writeRAM(0xFFFF, 0x23);
    m_memory.writeRAM(0x2345, 0xEA);

    m_cpu.setStackPointer(0xFD);
    m_cpu.setStatus(0x45);

    clock();                            // C1: Fetch BRK
    verifyRead(0x1000, 0x00);

    clock();                            // C2: Read padding byte
    verifyRead(0x1001, 0x42);

    clock();                            // C3: Push PCH of $1002
    verifyWrite(0x01FD, 0x10);

    clock();                            // C4: Push PCL of $1002
    verifyWrite(0x01FC, 0x02);

    clock();                            // C5: Push P with B + unused set
    verifyWrite(0x01FB, 0x75);

    clock();                            // C6: Vector low
    verifyRead(0xFFFE, 0x45);

    clock();                            // C7: Vector high
    verifyRead(0xFFFF, 0x23);

    clock();                            // Fetch first handler opcode
    verifyRead(0x2345, 0xEA);
}
void MOS6510TestBusCycles::testBrkStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x00);
    m_memory.writeRAM(0x1001, 0x42);

    m_memory.writeRAM(0xFFFE, 0x45);
    m_memory.writeRAM(0xFFFF, 0x23);
    m_memory.writeRAM(0x2345, 0xEA);

    m_cpu.setStackPointer(0x01);
    m_cpu.setStatus(0x45);

    clock();
    verifyRead(0x1000, 0x00);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyWrite(0x0101, 0x10);

    clock();
    verifyWrite(0x0100, 0x02);

    clock();
    verifyWrite(0x01FF, 0x75);

    clock();
    verifyRead(0xFFFE, 0x45);

    clock();
    verifyRead(0xFFFF, 0x23);

    clock();
    verifyRead(0x2345, 0xEA);
}
void MOS6510TestBusCycles::testClc()
{
    m_memory.writeRAM(0x1000, 0x18);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0x18);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testCld()
{
    m_memory.writeRAM(0x1000, 0xD8);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0xD8);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testCli()
{
    m_memory.writeRAM(0x1000, 0x58);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0x58);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testClv()
{
    m_memory.writeRAM(0x1000, 0xB8);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0xB8);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testSec()
{
    m_memory.writeRAM(0x1000, 0x38);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0x38);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testSed()
{
    m_memory.writeRAM(0x1000, 0xF8);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0xF8);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testSei()
{
    m_memory.writeRAM(0x1000, 0x78);
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0x78);

    clock();
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}
void MOS6510TestBusCycles::testCmpImmediate()
{
    m_memory.writeRAM(0x1000, 0xC9);    // CMP #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xC9);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testCmpZeroPage()
{
    m_memory.writeRAM(0x1000, 0xC5);    // CMP $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xC5);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testCmpZeroPageX()
{
    m_memory.writeRAM(0x1000, 0xD5);    // CMP $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);    // Dummy read
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD5);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCmpZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xD5);    // CMP $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD5);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testCmpAbsolute()
{
    m_memory.writeRAM(0x1000, 0xCD);    // CMP $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xCD);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testCmpAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xDD);    // CMP $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xDD);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testCmpAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xDD);    // CMP $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xDD);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testCmpAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xD9);    // CMP $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD9);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testCmpAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xD9);    // CMP $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD9);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testCmpIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0xC1);    // CMP ($20,X)
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x45);
    m_memory.writeRAM(0x0026, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xC1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x45);

    clock();
    verifyRead(0x0026, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCmpIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0xC1);    // CMP ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);
    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);
    m_memory.writeRAM(0x0000, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0xC1);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();
    verifyRead(0x00FC, 0x11);

    clock();
    verifyRead(0x00FF, 0x45);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testCmpIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xD1);    // CMP ($20),Y
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x40);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x40);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCmpIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xD1);    // CMP ($20),Y
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCmpIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0xD1);    // CMP ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x00FF, 0x40);
    m_memory.writeRAM(0x0000, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD1);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x40);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testCpxImmediate()
{
    m_memory.writeRAM(0x1000, 0xE0);    // CPX #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xE0);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCpxZeroPage()
{
    m_memory.writeRAM(0x1000, 0xE4);    // CPX $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xE4);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCpxAbsolute()
{
    m_memory.writeRAM(0x1000, 0xEC);    // CPX $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xEC);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testCpyImmediate()
{
    m_memory.writeRAM(0x1000, 0xC0);    // CPY #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xC0);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCpyZeroPage()
{
    m_memory.writeRAM(0x1000, 0xC4);    // CPY $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xC4);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testCpyAbsolute()
{
    m_memory.writeRAM(0x1000, 0xCC);    // CPY $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xCC);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testDecZeroPage()
{
    m_memory.writeRAM(0x1000, 0xC6);    // DEC $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xC6);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Read old
    verifyRead(0x0020, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x0020, 0x42);

    clock();                            // Write new
    verifyWrite(0x0020, 0x41);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testDecZeroPageX()
{
    m_memory.writeRAM(0x1000, 0xD6);    // DEC $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD6);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();                            // Read old
    verifyRead(0x0025, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x0025, 0x42);

    clock();                            // Write new
    verifyWrite(0x0025, 0x41);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testDecZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xD6);    // DEC $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xD6);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyWrite(0x0003, 0x41);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testDecAbsolute()
{
    m_memory.writeRAM(0x1000, 0xCE);    // DEC $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xCE);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyWrite(0x2345, 0x42);

    clock();
    verifyWrite(0x2345, 0x41);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testDecAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xDE);    // DEC $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xDE);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Indexed dummy read
    verifyRead(0x2345, 0x42);

    clock();                            // Read old
    verifyRead(0x2345, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x2345, 0x42);

    clock();                            // Write new
    verifyWrite(0x2345, 0x41);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testDecAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xDE);    // DEC $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xDE);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // Read old
    verifyRead(0x2403, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x2403, 0x42);

    clock();                            // Write new
    verifyWrite(0x2403, 0x41);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testDex()
{
    m_memory.writeRAM(0x1000, 0xCA);    // DEX
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch DEX
    verifyRead(0x1000, 0xCA);

    clock();                            // C2: Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}

void MOS6510TestBusCycles::testDey()
{
    m_memory.writeRAM(0x1000, 0x88);    // DEY
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0x88);

    clock();                            // Dummy read
    verifyRead(0x1001, 0xEA);

    clock();
    verifyRead(0x1001, 0xEA);
}
void MOS6510TestBusCycles::testEorImmediate()
{
    m_memory.writeRAM(0x1000, 0x49);    // EOR #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x49);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testEorZeroPage()
{
    m_memory.writeRAM(0x1000, 0x45);    // EOR $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x45);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testEorZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x55);    // EOR $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x55);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testEorZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x55);    // EOR $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x55);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testEorAbsolute()
{
    m_memory.writeRAM(0x1000, 0x4D);    // EOR $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0x4D);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testEorAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x5D);    // EOR $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x5D);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testEorAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x5D);    // EOR $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x5D);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testEorAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x59);    // EOR $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x59);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testEorAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x59);    // EOR $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x59);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testEorIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0x41);    // EOR ($20,X)
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x45);
    m_memory.writeRAM(0x0026, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x41);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x45);

    clock();
    verifyRead(0x0026, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testEorIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0x41);    // EOR ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);
    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);
    m_memory.writeRAM(0x0000, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0x41);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();
    verifyRead(0x00FC, 0x11);

    clock();
    verifyRead(0x00FF, 0x45);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testEorIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x51);    // EOR ($20),Y
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x40);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x51);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x40);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testEorIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x51);    // EOR ($20),Y
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x51);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testEorIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0x51);    // EOR ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x00FF, 0x40);
    m_memory.writeRAM(0x0000, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x51);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x40);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testIncZeroPage()
{
    m_memory.writeRAM(0x1000, 0xE6);    // INC $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x41);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xE6);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Read old
    verifyRead(0x0020, 0x41);

    clock();                            // Dummy write old
    verifyWrite(0x0020, 0x41);

    clock();                            // Write new
    verifyWrite(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testIncZeroPageX()
{
    m_memory.writeRAM(0x1000, 0xF6);    // INC $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x41);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xF6);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();                            // Read old
    verifyRead(0x0025, 0x41);

    clock();                            // Dummy write old
    verifyWrite(0x0025, 0x41);

    clock();                            // Write new
    verifyWrite(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testIncZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xF6);    // INC $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x41);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xF6);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read
    verifyRead(0x00FE, 0x11);

    clock();                            // Read old
    verifyRead(0x0003, 0x41);

    clock();                            // Dummy write old
    verifyWrite(0x0003, 0x41);

    clock();                            // Write new
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
void MOS6510TestBusCycles::testIncAbsolute()
{
    m_memory.writeRAM(0x1000, 0xEE);    // INC $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x41);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xEE);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Read old
    verifyRead(0x2345, 0x41);

    clock();                            // Dummy write old
    verifyWrite(0x2345, 0x41);

    clock();                            // Write new
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testIncAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xFE);    // INC $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x41);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0xFE);

    clock();                            // C2
    verifyRead(0x1001, 0x40);

    clock();                            // C3
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Indexed dummy read
    verifyRead(0x2345, 0x41);

    clock();                            // C5: Read old
    verifyRead(0x2345, 0x41);

    clock();                            // C6: Dummy write old
    verifyWrite(0x2345, 0x41);

    clock();                            // C7: Write new
    verifyWrite(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testIncAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xFE);    // INC $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x2403, 0x41);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0xFE);

    clock();                            // C2
    verifyRead(0x1001, 0xFE);

    clock();                            // C3
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Read old
    verifyRead(0x2403, 0x41);

    clock();                            // C6: Dummy write old
    verifyWrite(0x2403, 0x41);

    clock();                            // C7: Write new
    verifyWrite(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
void MOS6510TestBusCycles::testInx()
{
    m_memory.writeRAM(0x1000, 0xE8);    // INX
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0xE8);

    clock();                            // Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}
void MOS6510TestBusCycles::testIny()
{
    m_memory.writeRAM(0x1000, 0xC8);    // INY
    m_memory.writeRAM(0x1001, 0xEA);

    clock();
    verifyRead(0x1000, 0xC8);

    clock();                            // Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}
// -----------------------------------------------------------------------------
// JMP
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testJmpAbsolute()
{
    m_memory.writeRAM(0x1000, 0x4C);    // JMP $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x4C);

    clock();                            // C2: Read target low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Read target high
    verifyRead(0x1002, 0x23);

    clock();                            // Fetch first opcode at target
    verifyRead(0x2345, 0xEA);
}


void MOS6510TestBusCycles::testJmpIndirect()
{
    m_memory.writeRAM(0x1000, 0x6C);    // JMP ($2345)
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2345, 0x78);    // Target low
    m_memory.writeRAM(0x2346, 0x56);    // Target high

    m_memory.writeRAM(0x5678, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x6C);

    clock();                            // C2: Read pointer low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Read pointer high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Read target low
    verifyRead(0x2345, 0x78);

    clock();                            // C5: Read target high
    verifyRead(0x2346, 0x56);

    clock();                            // Fetch first opcode at target
    verifyRead(0x5678, 0xEA);
}


void MOS6510TestBusCycles::testJmpIndirectPageBoundaryBug()
{
    m_memory.writeRAM(0x1000, 0x6C);    // JMP ($23FF)
    m_memory.writeRAM(0x1001, 0xFF);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x23FF, 0x78);    // Target low
    m_memory.writeRAM(0x2300, 0x56);    // Target high due to NMOS bug

    // Deliberately different value at the address a "fixed"
    // implementation would use.
    m_memory.writeRAM(0x2400, 0x11);

    m_memory.writeRAM(0x5678, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x6C);

    clock();                            // C2: Read pointer low
    verifyRead(0x1001, 0xFF);

    clock();                            // C3: Read pointer high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Read target low
    verifyRead(0x23FF, 0x78);

    clock();                            // C5: High byte wraps within page
    verifyRead(0x2300, 0x56);

    clock();                            // Fetch first opcode at target
    verifyRead(0x5678, 0xEA);
}


// -----------------------------------------------------------------------------
// JSR
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testJsr()
{
    m_memory.writeRAM(0x1000, 0x20);    // JSR $2345
    m_memory.writeRAM(0x1001, 0x45);    // Target low
    m_memory.writeRAM(0x1002, 0x23);    // Target high

    m_memory.writeRAM(0x01FD, 0x11);    // Distinct dummy-read value
    m_memory.writeRAM(0x2345, 0xEA);

    m_cpu.setStackPointer(0xFD);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x20);

    clock();                            // C2: Read target low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Dummy read from stack
    verifyRead(0x01FD, 0x11);

    clock();                            // C4: Push return address high
    verifyWrite(0x01FD, 0x10);

    clock();                            // C5: Push return address low
    verifyWrite(0x01FC, 0x02);

    clock();                            // C6: Read target high
    verifyRead(0x1002, 0x23);

    clock();                            // Fetch first opcode at target
    verifyRead(0x2345, 0xEA);
}


void MOS6510TestBusCycles::testJsrStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x20);    // JSR $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x0100, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x2345, 0xEA);

    m_cpu.setStackPointer(0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x20);

    clock();                            // C2: Read target low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Dummy read from stack
    verifyRead(0x0100, 0x11);

    clock();                            // C4: Push return address high
    verifyWrite(0x0100, 0x10);

    clock();                            // C5: SP wraps, push low
    verifyWrite(0x01FF, 0x02);

    clock();                            // C6: Read target high
    verifyRead(0x1002, 0x23);

    clock();                            // Fetch first opcode at target
    verifyRead(0x2345, 0xEA);
}
// -----------------------------------------------------------------------------
// LDA
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testLdaImmediate()
{
    m_memory.writeRAM(0x1000, 0xA9);    // LDA #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xA9);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaZeroPage()
{
    m_memory.writeRAM(0x1000, 0xA5);    // LDA $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xA5);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaZeroPageX()
{
    m_memory.writeRAM(0x1000, 0xB5);    // LDA $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB5);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read of unindexed address
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xB5);    // LDA $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB5);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read before indexing
    verifyRead(0x00FE, 0x11);

    clock();                            // $FE + $05 wraps to $03
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaAbsolute()
{
    m_memory.writeRAM(0x1000, 0xAD);    // LDA $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xAD);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdaAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xBD);    // LDA $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBD);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdaAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xBD);    // LDA $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBD);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Low byte already indexed,
    // high byte still old
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdaAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xB9);    // LDA $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB9);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdaAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xB9);    // LDA $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB9);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdaIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0xA1);    // LDA ($20,X)
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x45);    // Pointer low
    m_memory.writeRAM(0x0026, 0x23);    // Pointer high

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xA1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read before adding X
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x45);

    clock();
    verifyRead(0x0026, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0xA1);    // LDA ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);

    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0xA1);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();
    verifyRead(0x00FC, 0x11);

    clock();
    verifyRead(0x00FF, 0x45);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xB1);    // LDA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x40);    // Pointer low
    m_memory.writeRAM(0x0021, 0x23);    // Pointer high

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x40);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xB1);    // LDA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdaIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0xB1);    // LDA ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);

    m_memory.writeRAM(0x00FF, 0x40);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB1);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x40);

    clock();
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// LDX
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testLdxImmediate()
{
    m_memory.writeRAM(0x1000, 0xA2);    // LDX #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xA2);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdxZeroPage()
{
    m_memory.writeRAM(0x1000, 0xA6);    // LDX $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xA6);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdxZeroPageY()
{
    m_memory.writeRAM(0x1000, 0xB6);    // LDX $20,Y
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB6);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdxZeroPageYWrapAround()
{
    m_memory.writeRAM(0x1000, 0xB6);    // LDX $FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB6);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdxAbsolute()
{
    m_memory.writeRAM(0x1000, 0xAE);    // LDX $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xAE);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdxAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xBE);    // LDX $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBE);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdxAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xBE);    // LDX $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBE);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// LDY
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testLdyImmediate()
{
    m_memory.writeRAM(0x1000, 0xA0);    // LDY #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xA0);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdyZeroPage()
{
    m_memory.writeRAM(0x1000, 0xA4);    // LDY $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0xA4);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdyZeroPageX()
{
    m_memory.writeRAM(0x1000, 0xB4);    // LDY $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB4);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdyZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xB4);    // LDY $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB4);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLdyAbsolute()
{
    m_memory.writeRAM(0x1000, 0xAC);    // LDY $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0xAC);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdyAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xBC);    // LDY $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBC);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLdyAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xBC);    // LDY $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBC);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
// -----------------------------------------------------------------------------
// LSR
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testLsrAccumulator()
{
    m_memory.writeRAM(0x1000, 0x4A);    // LSR A
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x4A);

    clock();                            // C2: Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testLsrZeroPage()
{
    m_memory.writeRAM(0x1000, 0x46);    // LSR $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x46);

    clock();                            // C2: Read address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Read old
    verifyRead(0x0020, 0x42);

    clock();                            // C4: Dummy write old
    verifyWrite(0x0020, 0x42);

    clock();                            // C5: Write new
    verifyWrite(0x0020, 0x21);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLsrZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x56);    // LSR $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x56);

    clock();                            // C2: Read base address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read before indexing
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Read old
    verifyRead(0x0025, 0x42);

    clock();                            // C5: Dummy write old
    verifyWrite(0x0025, 0x42);

    clock();                            // C6: Write new
    verifyWrite(0x0025, 0x21);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLsrZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x56);    // LSR $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x56);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read at unindexed address
    verifyRead(0x00FE, 0x11);

    clock();                            // $FE + $05 wraps to $03
    verifyRead(0x0003, 0x42);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyWrite(0x0003, 0x21);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testLsrAbsolute()
{
    m_memory.writeRAM(0x1000, 0x4E);    // LSR $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x4E);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Read old
    verifyRead(0x2345, 0x42);

    clock();                            // C5: Dummy write old
    verifyWrite(0x2345, 0x42);

    clock();                            // C6: Write new
    verifyWrite(0x2345, 0x21);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLsrAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x5E);    // LSR $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x5E);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x40);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Indexed dummy read
    verifyRead(0x2345, 0x42);

    clock();                            // C5: Read old
    verifyRead(0x2345, 0x42);

    clock();                            // C6: Dummy write old
    verifyWrite(0x2345, 0x42);

    clock();                            // C7: Write new
    verifyWrite(0x2345, 0x21);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testLsrAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x5E);    // LSR $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x5E);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0xFE);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Read old
    verifyRead(0x2403, 0x42);

    clock();                            // C6: Dummy write old
    verifyWrite(0x2403, 0x42);

    clock();                            // C7: Write new
    verifyWrite(0x2403, 0x21);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// NOP
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testNop()
{
    m_memory.writeRAM(0x1000, 0xEA);    // NOP
    m_memory.writeRAM(0x1001, 0xA9);    // Next opcode

    clock();                            // C1: Fetch NOP
    verifyRead(0x1000, 0xEA);

    clock();                            // C2: Dummy read
    verifyRead(0x1001, 0xA9);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xA9);
}


// -----------------------------------------------------------------------------
// ORA
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testOraImmediate()
{
    m_memory.writeRAM(0x1000, 0x09);    // ORA #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x09);

    clock();
    verifyRead(0x1001, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraZeroPage()
{
    m_memory.writeRAM(0x1000, 0x05);    // ORA $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();
    verifyRead(0x1000, 0x05);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x15);    // ORA $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x15);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x15);    // ORA $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x15);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraAbsolute()
{
    m_memory.writeRAM(0x1000, 0x0D);    // ORA $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();
    verifyRead(0x1000, 0x0D);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testOraAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x1D);    // ORA $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x1D);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testOraAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x1D);    // ORA $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x1D);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testOraAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x19);    // ORA $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x19);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testOraAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x19);    // ORA $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x19);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testOraIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0x01);    // ORA ($20,X)
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x45);    // Pointer low
    m_memory.writeRAM(0x0026, 0x23);    // Pointer high

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x01);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read before adding X
    verifyRead(0x0020, 0x11);

    clock();
    verifyRead(0x0025, 0x45);

    clock();
    verifyRead(0x0026, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0x01);    // ORA ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);

    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0x01);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();                            // Dummy read
    verifyRead(0x00FC, 0x11);

    clock();
    verifyRead(0x00FF, 0x45);

    clock();                            // $00FF + 1 wraps inside zero page
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x11);    // ORA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x40);    // Pointer low
    m_memory.writeRAM(0x0021, 0x23);    // Pointer high

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x11);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0x40);

    clock();
    verifyRead(0x0021, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x11);    // ORA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x11);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testOraIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0x11);    // ORA ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);

    m_memory.writeRAM(0x00FF, 0x40);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x11);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();
    verifyRead(0x00FF, 0x40);

    clock();                            // Pointer high wraps to $00
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
// -----------------------------------------------------------------------------
// PHA
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testPha()
{
    m_memory.writeRAM(0x1000, 0x48);    // PHA
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setStackPointer(0xFD);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x48);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Push A
    verifyWrite(0x01FD, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testPhaStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x48);    // PHA
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setStackPointer(0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x48);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Push at $0100
    verifyWrite(0x0100, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


// -----------------------------------------------------------------------------
// PHP
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testPhp()
{
    m_memory.writeRAM(0x1000, 0x08);    // PHP
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setStatus(0x45);
    m_cpu.setStackPointer(0xFD);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x08);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Push P with B + unused set
    verifyWrite(0x01FD, 0x75);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testPhpStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x08);    // PHP
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setStatus(0x45);
    m_cpu.setStackPointer(0x00);

    clock();
    verifyRead(0x1000, 0x08);

    clock();                            // Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Push at $0100
    verifyWrite(0x0100, 0x75);

    clock();
    verifyRead(0x1001, 0xEA);
}


// -----------------------------------------------------------------------------
// PLA
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testPla()
{
    m_memory.writeRAM(0x1000, 0x68);    // PLA
    m_memory.writeRAM(0x1001, 0xEA);

    // SP initially points below the value to pull.
    m_memory.writeRAM(0x01FC, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x01FD, 0x42);    // Value to pull

    m_cpu.setStackPointer(0xFC);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x68);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Dummy read at current SP
    verifyRead(0x01FC, 0x11);

    clock();                            // C4: SP incremented, pull A
    verifyRead(0x01FD, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testPlaStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x68);    // PLA
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FF, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0100, 0x42);    // Value after SP wraparound

    m_cpu.setStackPointer(0xFF);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x68);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Dummy read at SP=$FF
    verifyRead(0x01FF, 0x11);

    clock();                            // C4: SP $FF -> $00, pull
    verifyRead(0x0100, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


// -----------------------------------------------------------------------------
// PLP
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testPlp()
{
    m_memory.writeRAM(0x1000, 0x28);    // PLP
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FC, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x01FD, 0x45);    // Status value to pull

    m_cpu.setStackPointer(0xFC);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x28);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Dummy read at current SP
    verifyRead(0x01FC, 0x11);

    clock();                            // C4: SP incremented, pull P
    verifyRead(0x01FD, 0x45);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testPlpStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x28);    // PLP
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FF, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0100, 0x45);    // Status after SP wraparound

    m_cpu.setStackPointer(0xFF);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x28);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Dummy read at SP=$FF
    verifyRead(0x01FF, 0x11);

    clock();                            // C4: SP $FF -> $00, pull P
    verifyRead(0x0100, 0x45);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}
// -----------------------------------------------------------------------------
// ROL
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testRolAccumulator()
{
    m_memory.writeRAM(0x1000, 0x2A);    // ROL A
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setStatus(0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x2A);

    clock();                            // C2: Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testRolZeroPage()
{
    m_memory.writeRAM(0x1000, 0x26);    // ROL $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x21);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x00);              // Carry clear

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x26);

    clock();                            // C2: Read address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Read old
    verifyRead(0x0020, 0x21);

    clock();                            // C4: Dummy write old
    verifyWrite(0x0020, 0x21);

    clock();                            // C5: Write new
    verifyWrite(0x0020, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testRolZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x36);    // ROL $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x21);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x36);

    clock();                            // C2: Read base address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Read old
    verifyRead(0x0025, 0x21);

    clock();                            // C5: Dummy write old
    verifyWrite(0x0025, 0x21);

    clock();                            // C6: Write new
    verifyWrite(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testRolZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x36);    // ROL $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x21);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x36);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read before indexing
    verifyRead(0x00FE, 0x11);

    clock();                            // Read old after ZP wrap
    verifyRead(0x0003, 0x21);

    clock();
    verifyWrite(0x0003, 0x21);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testRolAbsolute()
{
    m_memory.writeRAM(0x1000, 0x2E);    // ROL $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x21);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x2E);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Read old
    verifyRead(0x2345, 0x21);

    clock();                            // Dummy write old
    verifyWrite(0x2345, 0x21);

    clock();                            // Write new
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testRolAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x3E);    // ROL $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x21);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();                            // C1
    verifyRead(0x1000, 0x3E);

    clock();                            // C2
    verifyRead(0x1001, 0x40);

    clock();                            // C3
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Indexed dummy read
    verifyRead(0x2345, 0x21);

    clock();                            // C5: Read old
    verifyRead(0x2345, 0x21);

    clock();                            // C6: Dummy write old
    verifyWrite(0x2345, 0x21);

    clock();                            // C7: Write new
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testRolAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x3E);    // ROL $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x2403, 0x21);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x3E);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Read old
    verifyRead(0x2403, 0x21);

    clock();                            // C6: Dummy write old
    verifyWrite(0x2403, 0x21);

    clock();                            // C7: Write new
    verifyWrite(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// ROR
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testRorAccumulator()
{
    m_memory.writeRAM(0x1000, 0x6A);    // ROR A
    m_memory.writeRAM(0x1001, 0xEA);

    m_cpu.setStatus(0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x6A);

    clock();                            // C2: Dummy read
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testRorZeroPage()
{
    m_memory.writeRAM(0x1000, 0x66);    // ROR $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setStatus(0x00);              // Carry clear

    clock();
    verifyRead(0x1000, 0x66);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Read old
    verifyRead(0x0020, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x0020, 0x42);

    clock();                            // Write new
    verifyWrite(0x0020, 0x21);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testRorZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x76);    // ROR $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x76);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read
    verifyRead(0x0020, 0x11);

    clock();                            // Read old
    verifyRead(0x0025, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x0025, 0x42);

    clock();                            // Write new
    verifyWrite(0x0025, 0x21);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testRorZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x76);    // ROR $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x76);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x00FE, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyWrite(0x0003, 0x21);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testRorAbsolute()
{
    m_memory.writeRAM(0x1000, 0x6E);    // ROR $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x6E);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Read old
    verifyRead(0x2345, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x2345, 0x42);

    clock();                            // Write new
    verifyWrite(0x2345, 0x21);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testRorAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x7E);    // ROR $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x7E);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Indexed dummy read
    verifyRead(0x2345, 0x42);

    clock();                            // Read old
    verifyRead(0x2345, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x2345, 0x42);

    clock();                            // Write new
    verifyWrite(0x2345, 0x21);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testRorAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x7E);    // ROR $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setStatus(0x00);

    clock();
    verifyRead(0x1000, 0x7E);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // Read old
    verifyRead(0x2403, 0x42);

    clock();                            // Dummy write old
    verifyWrite(0x2403, 0x42);

    clock();                            // Write new
    verifyWrite(0x2403, 0x21);

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// RTI
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testRti()
{
    m_memory.writeRAM(0x1000, 0x40);    // RTI
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FA, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x01FB, 0x45);    // P
    m_memory.writeRAM(0x01FC, 0x78);    // PCL
    m_memory.writeRAM(0x01FD, 0x56);    // PCH

    m_memory.writeRAM(0x5678, 0xEA);

    m_cpu.setStackPointer(0xFA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x40);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Dummy read at current SP
    verifyRead(0x01FA, 0x11);

    clock();                            // C4: Pull P
    verifyRead(0x01FB, 0x45);

    clock();                            // C5: Pull PCL
    verifyRead(0x01FC, 0x78);

    clock();                            // C6: Pull PCH
    verifyRead(0x01FD, 0x56);

    clock();                            // Fetch opcode at restored PC
    verifyRead(0x5678, 0xEA);
}


void MOS6510TestBusCycles::testRtiStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x40);    // RTI
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FE, 0x11);    // Dummy read
    m_memory.writeRAM(0x01FF, 0x45);    // P
    m_memory.writeRAM(0x0100, 0x78);    // PCL
    m_memory.writeRAM(0x0101, 0x56);    // PCH

    m_memory.writeRAM(0x5678, 0xEA);

    m_cpu.setStackPointer(0xFE);

    clock();
    verifyRead(0x1000, 0x40);

    clock();                            // Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Dummy read at SP=$FE
    verifyRead(0x01FE, 0x11);

    clock();                            // SP $FE -> $FF, pull P
    verifyRead(0x01FF, 0x45);

    clock();                            // SP $FF -> $00, pull PCL
    verifyRead(0x0100, 0x78);

    clock();                            // SP $00 -> $01, pull PCH
    verifyRead(0x0101, 0x56);

    clock();
    verifyRead(0x5678, 0xEA);
}


// -----------------------------------------------------------------------------
// RTS
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testRts()
{
    m_memory.writeRAM(0x1000, 0x60);    // RTS
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FB, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x01FC, 0x77);    // Return address low
    m_memory.writeRAM(0x01FD, 0x56);    // Return address high

    // Pulled address = $5677.
    // RTS increments it to $5678.
    m_memory.writeRAM(0x5677, 0x22);    // Final dummy read
    m_memory.writeRAM(0x5678, 0xEA);

    m_cpu.setStackPointer(0xFB);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x60);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // C3: Dummy read at current SP
    verifyRead(0x01FB, 0x11);

    clock();                            // C4: Pull PCL
    verifyRead(0x01FC, 0x77);

    clock();                            // C5: Pull PCH
    verifyRead(0x01FD, 0x56);

    clock();                            // C6: Dummy read at pulled address
    //     while incrementing PC
    verifyRead(0x5677, 0x22);

    clock();                            // Fetch next opcode at return PC + 1
    verifyRead(0x5678, 0xEA);
}


void MOS6510TestBusCycles::testRtsStackPointerWrapAround()
{
    m_memory.writeRAM(0x1000, 0x60);    // RTS
    m_memory.writeRAM(0x1001, 0xEA);

    m_memory.writeRAM(0x01FE, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x01FF, 0x77);    // Return address low
    m_memory.writeRAM(0x0100, 0x56);    // Return address high

    m_memory.writeRAM(0x5677, 0x22);
    m_memory.writeRAM(0x5678, 0xEA);

    m_cpu.setStackPointer(0xFE);

    clock();
    verifyRead(0x1000, 0x60);

    clock();                            // Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Dummy read at SP=$FE
    verifyRead(0x01FE, 0x11);

    clock();                            // SP $FE -> $FF, pull PCL
    verifyRead(0x01FF, 0x77);

    clock();                            // SP $FF -> $00, pull PCH
    verifyRead(0x0100, 0x56);

    clock();                            // Dummy read before PC increment
    verifyRead(0x5677, 0x22);

    clock();
    verifyRead(0x5678, 0xEA);
}
// -----------------------------------------------------------------------------
// SBC
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testSbcImmediate()
{
    m_memory.writeRAM(0x1000, 0xE9);    // SBC #$42
    m_memory.writeRAM(0x1001, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xE9);

    clock();                            // C2: Read operand
    verifyRead(0x1001, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcZeroPage()
{
    m_memory.writeRAM(0x1000, 0xE5);    // SBC $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xE5);

    clock();                            // C2: Read address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Read operand
    verifyRead(0x0020, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcZeroPageX()
{
    m_memory.writeRAM(0x1000, 0xF5);    // SBC $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xF5);

    clock();                            // C2: Read base address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read before indexing
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Read operand
    verifyRead(0x0025, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0xF5);    // SBC $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xF5);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read at unindexed address
    verifyRead(0x00FE, 0x11);

    clock();                            // $FE + $05 wraps to $03
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcAbsolute()
{
    m_memory.writeRAM(0x1000, 0xED);    // SBC $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xED);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Read operand
    verifyRead(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testSbcAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xFD);    // SBC $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xFD);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x40);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Read operand
    verifyRead(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testSbcAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xFD);    // SBC $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xFD);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0xFE);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Read operand
    verifyRead(0x2403, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testSbcAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xF9);    // SBC $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xF9);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x40);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Read operand
    verifyRead(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testSbcAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xF9);    // SBC $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xF9);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();                            // Corrected-page operand read
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testSbcIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0xE1);    // SBC ($20,X)
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x45);    // Pointer low
    m_memory.writeRAM(0x0026, 0x23);    // Pointer high

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xE1);

    clock();                            // C2: Read ZP address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read before adding X
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Read pointer low
    verifyRead(0x0025, 0x45);

    clock();                            // C5: Read pointer high
    verifyRead(0x0026, 0x23);

    clock();                            // C6: Read operand
    verifyRead(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0xE1);    // SBC ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);

    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0xE1);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();                            // Dummy read before adding X
    verifyRead(0x00FC, 0x11);

    clock();                            // Pointer low at $FF
    verifyRead(0x00FF, 0x45);

    clock();                            // Pointer high wraps to $00
    verifyRead(0x0000, 0x23);

    clock();
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xF1);    // SBC ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x40);    // Pointer low
    m_memory.writeRAM(0x0021, 0x23);    // Pointer high

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xF1);

    clock();                            // C2: Read ZP pointer address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Read pointer low
    verifyRead(0x0020, 0x40);

    clock();                            // C4: Read pointer high
    verifyRead(0x0021, 0x23);

    clock();                            // C5: Read operand
    verifyRead(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0xF1);    // SBC ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);    // Pointer low
    m_memory.writeRAM(0x0021, 0x23);    // Pointer high

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x2403, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xF1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();                            // C5: Wrong-page read
    verifyRead(0x2303, 0x11);

    clock();                            // C6: Corrected-page operand read
    verifyRead(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testSbcIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0xF1);    // SBC ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);

    m_memory.writeRAM(0x00FF, 0x40);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x2345, 0x42);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xF1);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();                            // Pointer low
    verifyRead(0x00FF, 0x40);

    clock();                            // Pointer high wraps to $00
    verifyRead(0x0000, 0x23);

    clock();                            // Read operand
    verifyRead(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}
// -----------------------------------------------------------------------------
// STA
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testStaZeroPage()
{
    m_memory.writeRAM(0x1000, 0x85);    // STA $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Read address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Write A
    verifyWrite(0x0020, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x95);    // STA $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x95);

    clock();                            // C2: Read base address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read before indexing
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Write A
    verifyWrite(0x0025, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x95);    // STA $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0x95);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read at unindexed address
    verifyRead(0x00FE, 0x11);

    clock();                            // $FE + $05 wraps to $03
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaAbsolute()
{
    m_memory.writeRAM(0x1000, 0x8D);    // STA $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setAccumulator(0x42);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x8D);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x45);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Write A
    verifyWrite(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testStaAbsoluteXWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x9D);    // STA $2340,X
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x9D);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0x40);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Unconditional indexed dummy read
    verifyRead(0x2345, 0x11);

    clock();                            // C5: Write A
    verifyWrite(0x2345, 0x42);

    clock();                            // Fetch next opcode
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testStaAbsoluteXWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x9D);    // STA $23FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x9D);

    clock();                            // C2: Read low
    verifyRead(0x1001, 0xFE);

    clock();                            // C3: Read high
    verifyRead(0x1002, 0x23);

    clock();                            // C4: Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // C5: Write to corrected address
    verifyWrite(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testStaAbsoluteYWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x99);    // STA $2340,Y
    m_memory.writeRAM(0x1001, 0x40);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x2345, 0x11);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x99);

    clock();
    verifyRead(0x1001, 0x40);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Unconditional indexed dummy read
    verifyRead(0x2345, 0x11);

    clock();                            // Write A
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testStaAbsoluteYWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x99);    // STA $23FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0x23);

    m_memory.writeRAM(0x2303, 0x11);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x99);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // Write to corrected address
    verifyWrite(0x2403, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


void MOS6510TestBusCycles::testStaIndexedIndirect()
{
    m_memory.writeRAM(0x1000, 0x81);    // STA ($20,X)
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x0025, 0x45);    // Pointer low
    m_memory.writeRAM(0x0026, 0x23);    // Pointer high

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x81);

    clock();                            // C2: Read ZP address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Dummy read before adding X
    verifyRead(0x0020, 0x11);

    clock();                            // C4: Read pointer low
    verifyRead(0x0025, 0x45);

    clock();                            // C5: Read pointer high
    verifyRead(0x0026, 0x23);

    clock();                            // C6: Write A
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaIndexedIndirectWrapAround()
{
    m_memory.writeRAM(0x1000, 0x81);    // STA ($FC,X)
    m_memory.writeRAM(0x1001, 0xFC);

    m_memory.writeRAM(0x00FC, 0x11);
    m_memory.writeRAM(0x00FF, 0x45);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setXRegister(0x03);

    clock();
    verifyRead(0x1000, 0x81);

    clock();
    verifyRead(0x1001, 0xFC);

    clock();                            // Dummy read
    verifyRead(0x00FC, 0x11);

    clock();                            // Pointer low
    verifyRead(0x00FF, 0x45);

    clock();                            // Pointer high wraps to $00
    verifyRead(0x0000, 0x23);

    clock();
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaIndirectIndexedWithoutPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x91);    // STA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0x40);    // Pointer low
    m_memory.writeRAM(0x0021, 0x23);    // Pointer high
    m_memory.writeRAM(0x2345, 0x11);    // Dummy-read value

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setYRegister(0x05);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x91);

    clock();                            // C2: Read ZP pointer address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Read pointer low
    verifyRead(0x0020, 0x40);

    clock();                            // C4: Read pointer high
    verifyRead(0x0021, 0x23);

    clock();                            // C5: Unconditional indexed dummy read
    verifyRead(0x2345, 0x11);

    clock();                            // C6: Write A
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaIndirectIndexedWithPageCrossing()
{
    m_memory.writeRAM(0x1000, 0x91);    // STA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);    // Pointer low
    m_memory.writeRAM(0x0021, 0x23);    // Pointer high

    m_memory.writeRAM(0x2303, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x91);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0x23);

    clock();                            // Wrong-page dummy read
    verifyRead(0x2303, 0x11);

    clock();                            // Write to corrected address
    verifyWrite(0x2403, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStaIndirectIndexedWrapAround()
{
    m_memory.writeRAM(0x1000, 0x91);    // STA ($FF),Y
    m_memory.writeRAM(0x1001, 0xFF);

    m_memory.writeRAM(0x00FF, 0x40);    // Pointer low
    m_memory.writeRAM(0x0000, 0x23);    // Pointer high wraps in ZP
    m_memory.writeRAM(0x2345, 0x11);    // Dummy-read value

    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setAccumulator(0x42);
    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x91);

    clock();
    verifyRead(0x1001, 0xFF);

    clock();                            // Pointer low
    verifyRead(0x00FF, 0x40);

    clock();                            // Pointer high wraps to $00
    verifyRead(0x0000, 0x23);

    clock();                            // Unconditional indexed dummy read
    verifyRead(0x2345, 0x11);

    clock();
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


// -----------------------------------------------------------------------------
// STX
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testStxZeroPage()
{
    m_memory.writeRAM(0x1000, 0x86);    // STX $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x42);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x86);

    clock();                            // C2: Read address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Write X
    verifyWrite(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStxZeroPageY()
{
    m_memory.writeRAM(0x1000, 0x96);    // STX $20,Y
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x42);
    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x96);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read before indexing
    verifyRead(0x0020, 0x11);

    clock();                            // Write X
    verifyWrite(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStxZeroPageYWrapAround()
{
    m_memory.writeRAM(0x1000, 0x96);    // STX $FE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x42);
    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0x96);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read
    verifyRead(0x00FE, 0x11);

    clock();                            // $FE + $05 wraps to $03
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStxAbsolute()
{
    m_memory.writeRAM(0x1000, 0x8E);    // STX $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x42);

    clock();
    verifyRead(0x1000, 0x8E);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Write X
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}


// -----------------------------------------------------------------------------
// STY
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testStyZeroPage()
{
    m_memory.writeRAM(0x1000, 0x84);    // STY $20
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x42);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x84);

    clock();                            // C2: Read address
    verifyRead(0x1001, 0x20);

    clock();                            // C3: Write Y
    verifyWrite(0x0020, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStyZeroPageX()
{
    m_memory.writeRAM(0x1000, 0x94);    // STY $20,X
    m_memory.writeRAM(0x1001, 0x20);
    m_memory.writeRAM(0x0020, 0x11);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x42);

    clock();
    verifyRead(0x1000, 0x94);

    clock();
    verifyRead(0x1001, 0x20);

    clock();                            // Dummy read before indexing
    verifyRead(0x0020, 0x11);

    clock();                            // Write Y
    verifyWrite(0x0025, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStyZeroPageXWrapAround()
{
    m_memory.writeRAM(0x1000, 0x94);    // STY $FE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x00FE, 0x11);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setYRegister(0x42);

    clock();
    verifyRead(0x1000, 0x94);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();                            // Dummy read
    verifyRead(0x00FE, 0x11);

    clock();                            // $FE + $05 wraps to $03
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}


void MOS6510TestBusCycles::testStyAbsolute()
{
    m_memory.writeRAM(0x1000, 0x8C);    // STY $2345
    m_memory.writeRAM(0x1001, 0x45);
    m_memory.writeRAM(0x1002, 0x23);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x42);

    clock();
    verifyRead(0x1000, 0x8C);

    clock();
    verifyRead(0x1001, 0x45);

    clock();
    verifyRead(0x1002, 0x23);

    clock();                            // Write Y
    verifyWrite(0x2345, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}
// -----------------------------------------------------------------------------
// Transfer instructions
// -----------------------------------------------------------------------------

void MOS6510TestBusCycles::testTax()
{
    m_memory.writeRAM(0x1000, 0xAA);    // TAX
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xAA);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testTay()
{
    m_memory.writeRAM(0x1000, 0xA8);    // TAY
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xA8);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testTsx()
{
    m_memory.writeRAM(0x1000, 0xBA);    // TSX
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xBA);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testTxa()
{
    m_memory.writeRAM(0x1000, 0x8A);    // TXA
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x8A);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testTxs()
{
    m_memory.writeRAM(0x1000, 0x9A);    // TXS
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x9A);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}


void MOS6510TestBusCycles::testTya()
{
    m_memory.writeRAM(0x1000, 0x98);    // TYA
    m_memory.writeRAM(0x1001, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x98);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0xEA);

    clock();                            // Fetch next opcode
    verifyRead(0x1001, 0xEA);
}
void MOS6510TestBusCycles::testBccTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0x90);    // BCC -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);    // Dummy-read value
    m_memory.writeRAM(0x100A, 0xEA);    // Branch target

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatus(0x00);              // C clear -> taken

    clock();                            // C1
    verifyRead(0x1010, 0x90);

    clock();                            // C2
    verifyRead(0x1011, 0xF8);

    clock();                            // C3: Dummy read at PC after operand
    verifyRead(0x1012, 0x11);

    clock();                            // Next opcode at target
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBccTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0x90);    // BCC -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);    // Wrong-page dummy read
    m_memory.writeRAM(0x10FE, 0xEA);    // Branch target

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Carry, false);

    clock();
    verifyRead(0x1100, 0x90);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBcsTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0xB0);    // BCS -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Carry, true);

    clock();
    verifyRead(0x1010, 0xB0);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBcsTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0xB0);    // BCS -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Carry, true);

    clock();
    verifyRead(0x1100, 0xB0);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBeqTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0xF0);    // BEQ -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Zero, true);

    clock();
    verifyRead(0x1010, 0xF0);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBeqTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0xF0);    // BEQ -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Zero, true);

    clock();
    verifyRead(0x1100, 0xF0);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBmiTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0x30);    // BMI -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Negative, true);

    clock();
    verifyRead(0x1010, 0x30);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBmiTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0x30);    // BMI -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Negative, true);

    clock();
    verifyRead(0x1100, 0x30);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBneTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0xD0);    // BNE -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Zero, false);

    clock();
    verifyRead(0x1010, 0xD0);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBneTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0xD0);    // BNE -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Zero, false);

    clock();
    verifyRead(0x1100, 0xD0);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBplTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0x10);    // BPL -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Negative, false);

    clock();
    verifyRead(0x1010, 0x10);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBplTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0x10);    // BPL -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Negative, false);

    clock();
    verifyRead(0x1100, 0x10);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBvcTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0x50);    // BVC -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Overflow, false);

    clock();
    verifyRead(0x1010, 0x50);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBvcTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0x50);    // BVC -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Overflow, false);

    clock();
    verifyRead(0x1100, 0x50);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testBvsTakenBackwardWithoutPageCrossing()
{
    m_memory.writeRAM(0x1010, 0x70);    // BVS -$08
    m_memory.writeRAM(0x1011, 0xF8);
    m_memory.writeRAM(0x1012, 0x11);
    m_memory.writeRAM(0x100A, 0xEA);

    m_cpu.setProgramCounter(0x1010);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Overflow, true);

    clock();
    verifyRead(0x1010, 0x70);

    clock();
    verifyRead(0x1011, 0xF8);

    clock();
    verifyRead(0x1012, 0x11);

    clock();
    verifyRead(0x100A, 0xEA);
}

void MOS6510TestBusCycles::testBvsTakenBackwardWithPageCrossing()
{
    m_memory.writeRAM(0x1100, 0x70);    // BVS -$04
    m_memory.writeRAM(0x1101, 0xFC);
    m_memory.writeRAM(0x1102, 0x11);
    m_memory.writeRAM(0x11FE, 0x22);
    m_memory.writeRAM(0x10FE, 0xEA);

    m_cpu.setProgramCounter(0x1100);
    m_cpu.setStatusFlag(MOS6510StatusFlag::Overflow, true);

    clock();
    verifyRead(0x1100, 0x70);

    clock();
    verifyRead(0x1101, 0xFC);

    clock();
    verifyRead(0x1102, 0x11);

    clock();
    verifyRead(0x11FE, 0x22);

    clock();
    verifyRead(0x10FE, 0xEA);
}

void MOS6510TestBusCycles::testLdaImmediateProgramCounterWrapAround()
{
    m_memory.writeRAM(0xFFFF, 0xA9);    // LDA #$42
    m_memory.writeRAM(0x0000, 0x42);
    m_memory.writeRAM(0x0001, 0xEA);

    m_cpu.setProgramCounter(0xFFFF);

    clock();                            // C1: Fetch opcode
    verifyRead(0xFFFF, 0xA9);

    clock();                            // C2: Operand after PC wrap
    verifyRead(0x0000, 0x42);

    clock();                            // Next opcode
    verifyRead(0x0001, 0xEA);
}

void MOS6510TestBusCycles::testLdaAbsoluteXAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0xBD);    // LDA $FFFE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x0003, 0x42);    // Effective address after 16-bit wrap
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();
    verifyRead(0x1000, 0xBD);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0xFF);

    clock();
    verifyRead(0xFF03, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testLdaAbsoluteYAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0xB9);    // LDA $FFFE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);
    m_memory.writeRAM(0x0003, 0x42);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB9);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0xFF);

    clock();
    verifyRead(0xFF03, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testLdaIndirectIndexedAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0xB1);    // LDA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);    // Wrong-page read
    m_memory.writeRAM(0x0003, 0x42);    // Effective address
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);

    clock();
    verifyRead(0x1000, 0xB1);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0xFF);

    clock();
    verifyRead(0xFF03, 0x11);

    clock();
    verifyRead(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testStaAbsoluteXAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0x9D);    // STA $FFFE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);    // Dummy read
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);
    m_cpu.setAccumulator(0x42);

    clock();
    verifyRead(0x1000, 0x9D);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0xFF);

    clock();
    verifyRead(0xFF03, 0x11);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testStaAbsoluteYAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0x99);    // STA $FFFE,Y
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setYRegister(0x05);
    m_cpu.setAccumulator(0x42);

    clock();
    verifyRead(0x1000, 0x99);

    clock();
    verifyRead(0x1001, 0xFE);

    clock();
    verifyRead(0x1002, 0xFF);

    clock();
    verifyRead(0xFF03, 0x11);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testStaIndirectIndexedAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0x91);    // STA ($20),Y
    m_memory.writeRAM(0x1001, 0x20);

    m_memory.writeRAM(0x0020, 0xFE);
    m_memory.writeRAM(0x0021, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);
    m_memory.writeRAM(0x1002, 0xEA);

    m_cpu.setYRegister(0x05);
    m_cpu.setAccumulator(0x42);

    clock();
    verifyRead(0x1000, 0x91);

    clock();
    verifyRead(0x1001, 0x20);

    clock();
    verifyRead(0x0020, 0xFE);

    clock();
    verifyRead(0x0021, 0xFF);

    clock();
    verifyRead(0xFF03, 0x11);

    clock();
    verifyWrite(0x0003, 0x42);

    clock();
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestBusCycles::testIncAbsoluteXAddressSpaceWrapAround()
{
    m_memory.writeRAM(0x1000, 0xFE);    // INC $FFFE,X
    m_memory.writeRAM(0x1001, 0xFE);
    m_memory.writeRAM(0x1002, 0xFF);

    m_memory.writeRAM(0xFF03, 0x11);    // Wrong-page dummy read
    m_memory.writeRAM(0x0003, 0x41);    // Old value
    m_memory.writeRAM(0x1003, 0xEA);

    m_cpu.setXRegister(0x05);

    clock();                            // C1
    verifyRead(0x1000, 0xFE);

    clock();                            // C2
    verifyRead(0x1001, 0xFE);

    clock();                            // C3
    verifyRead(0x1002, 0xFF);

    clock();                            // C4: Indexed dummy read
    verifyRead(0xFF03, 0x11);

    clock();                            // C5: Read old value
    verifyRead(0x0003, 0x41);

    clock();                            // C6: Dummy write old value
    verifyWrite(0x0003, 0x41);

    clock();                            // C7: Write new value
    verifyWrite(0x0003, 0x42);

    clock();                            // Next opcode
    verifyRead(0x1003, 0xEA);
}

void MOS6510TestBusCycles::testRtsProgramCounterWrapAround()
{
    m_memory.writeRAM(0x1000, 0x60);    // RTS
    m_memory.writeRAM(0x1001, 0x11);    // Dummy-read value

    // SP starts at $FD.
    // RTS pulls $FFFF from $01FE/$01FF and then increments it to $0000.
    m_memory.writeRAM(0x01FD, 0x22);    // Dummy stack read
    m_memory.writeRAM(0x01FE, 0xFF);    // Return address low
    m_memory.writeRAM(0x01FF, 0xFF);    // Return address high

    m_memory.writeRAM(0xFFFF, 0x33);    // Final RTS dummy read
    m_memory.writeRAM(0x0000, 0xEA);    // Next opcode after PC wrap

    m_cpu.setStackPointer(0xFD);

    clock();                            // C1: Fetch RTS
    verifyRead(0x1000, 0x60);

    clock();                            // C2: Dummy read at PC
    verifyRead(0x1001, 0x11);

    clock();                            // C3: Dummy stack read
    verifyRead(0x01FD, 0x22);

    clock();                            // C4: Pull PCL
    verifyRead(0x01FE, 0xFF);

    clock();                            // C5: Pull PCH
    verifyRead(0x01FF, 0xFF);

    clock();                            // C6: Dummy read at pulled address
    verifyRead(0xFFFF, 0x33);

    clock();                            // Next opcode after RTS increments PC
    verifyRead(0x0000, 0xEA);
}
