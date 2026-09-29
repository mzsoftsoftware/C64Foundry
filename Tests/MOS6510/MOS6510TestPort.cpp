#include "MOS6510TestPort.h"

#include <QtTest>


MOS6510TestPort::MOS6510TestPort()
{
}

MOS6510TestPort::~MOS6510TestPort()
{
}


void MOS6510TestPort::testDataDirectionRegisterWrite()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x00);

    clock();                            // C3: Write DDR
    verifyWriteCycle(0x0000);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestPort::testDataDirectionRegisterRead()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    //
    // First write $2A to the data direction register.
    //
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    //
    // Then read the data direction register back.
    //
    m_memory.writeRAM(0x1002, 0xA5);    // LDA $00
    m_memory.writeRAM(0x1003, 0x00);
    m_memory.writeRAM(0x1004, 0xEA);

    clock();                            // STA C1
    verifyRead(0x1000, 0x85);

    clock();                            // STA C2
    verifyRead(0x1001, 0x00);

    clock();                            // STA C3
    verifyWriteCycle(0x0000);

    clock();                            // LDA C1
    verifyRead(0x1002, 0xA5);

    clock();                            // LDA C2
    verifyRead(0x1003, 0x00);

    clock();                            // LDA C3
    verifyReadCycle(0x0000);

    QCOMPARE(m_cpu.accumulator(), quint8(0x2A));

    clock();                            // Fetch next opcode
    verifyRead(0x1004, 0xEA);
}

void MOS6510TestPort::testDataRegisterWrite()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x15);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestPort::testDataRegisterReadOutputs()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);

    //
    // Configure P0-P5 as outputs.
    //
    m_cpu.setAccumulator(0x3F);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    //
    // Write a pattern to the port data register.
    //
    m_memory.writeRAM(0x1002, 0xA9);    // LDA #$15
    m_memory.writeRAM(0x1003, 0x15);
    m_memory.writeRAM(0x1004, 0x85);    // STA $01
    m_memory.writeRAM(0x1005, 0x01);

    //
    // Read the port data register.
    //
    m_memory.writeRAM(0x1006, 0xA5);    // LDA $01
    m_memory.writeRAM(0x1007, 0x01);
    m_memory.writeRAM(0x1008, 0xEA);

    clock();                            // STA $00 C1
    verifyRead(0x1000, 0x85);

    clock();                            // STA $00 C2
    verifyRead(0x1001, 0x00);

    clock();                            // STA $00 C3
    verifyWriteCycle(0x0000);

    clock();                            // LDA #$15 C1
    verifyRead(0x1002, 0xA9);

    clock();                            // LDA #$15 C2
    verifyRead(0x1003, 0x15);

    QCOMPARE(m_cpu.accumulator(), quint8(0x15));

    clock();                            // STA $01 C1
    verifyRead(0x1004, 0x85);

    clock();                            // STA $01 C2
    verifyRead(0x1005, 0x01);

    clock();                            // STA $01 C3
    verifyWriteCycle(0x0001);

    clock();                            // LDA $01 C1
    verifyRead(0x1006, 0xA5);

    clock();                            // LDA $01 C2
    verifyRead(0x1007, 0x01);

    clock();                            // LDA $01 C3
    verifyReadCycle(0x0001);

    QCOMPARE(m_cpu.accumulator(), quint8(0x15));

    clock();                            // Fetch next opcode
    verifyRead(0x1008, 0xEA);
}

void MOS6510TestPort::testDataDirectionRegisterWriteDataBusNotDriven()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x00);

    clock();
    verifyWriteCycle(0x0000);

    QVERIFY(!m_bus.cpuDrivesDataBus());
}
void MOS6510TestPort::testDataRegisterWriteDataBusNotDriven()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x15);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x01);

    clock();
    verifyWriteCycle(0x0001);

    QVERIFY(!m_bus.cpuDrivesDataBus());
}

void MOS6510TestPort::testDataDirectionRegisterWriteBusCycle()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x00);

    clock();                            // C3: Write DDR
    verifyWriteCycle(0x0000);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestPort::testDataRegisterWriteBusCycle()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x15);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);
    m_memory.writeRAM(0x1002, 0xEA);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    clock();                            // Fetch next opcode
    verifyRead(0x1002, 0xEA);
}

void MOS6510TestPort::testDataDirectionRegisterWritePreservesDataBusValue()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x00);

    //
    // Simulate an external bus master, for example the VIC-II,
    // leaving a value on the data bus before the CPU write phase.
    //
    m_bus.setDataBusValue(0xA5);

    clock();                            // C3: Write DDR
    verifyWriteCycle(0x0000);

    QVERIFY(!m_bus.cpuDrivesDataBus());
    QCOMPARE(m_bus.dataBusValue(), quint8(0xA5));
}

void MOS6510TestPort::testDataRegisterWritePreservesDataBusValue()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x15);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    //
    // Simulate an external bus master, for example the VIC-II,
    // leaving a value on the data bus before the CPU write phase.
    //
    m_bus.setDataBusValue(0x5A);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    QVERIFY(!m_bus.cpuDrivesDataBus());
    QCOMPARE(m_bus.dataBusValue(), quint8(0x5A));
}

void MOS6510TestPort::testDataDirectionRegisterWriteWritesDataBusValueToRAM()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x0000, 0x11);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x00);

    //
    // The 6510 does not drive D0-D7 when writing its internal
    // processor-port registers. Simulate the value already present
    // on the external data bus.
    //
    m_bus.setDataBusValue(0xA5);

    clock();                            // C3: Write DDR
    verifyWriteCycle(0x0000);

    QVERIFY(!m_bus.cpuDrivesDataBus());

    //
    // The internal register receives the CPU value, while the
    // physical RAM underneath receives the external bus value.
    //
    QVERIFY(!m_bus.cpuDrivesDataBus());
    QCOMPARE(m_memory.readRAM(0x0000), quint8(0xA5));
}

void MOS6510TestPort::testDataRegisterWriteWritesDataBusValueToRAM()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x15);

    m_memory.writeRAM(0x0001, 0x11);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    m_bus.setDataBusValue(0x5A);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    QVERIFY(!m_bus.cpuDrivesDataBus());
    QCOMPARE(m_memory.readRAM(0x0001), quint8(0x5A));
}

void MOS6510TestPort::testImmediateLoadPcWrap()
{
    setupCpu();

    //
    // Configure the data direction register so that reading
    // $0000 returns $2A.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x2A);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x00);

    clock();
    verifyWriteCycle(0x0000);

    //
    // Execute LDA #imm at $FFFF.
    //
    // The program counter wraps around after fetching the
    // opcode. Therefore the immediate operand is read from
    // the internal data direction register at $0000.
    //
    m_memory.writeRAM(0xFFFF, 0xA9);    // LDA #imm

    m_cpu.setProgramCounter(0xFFFF);

    clock();                            // C1: Opcode fetch
    verifyRead(0xFFFF, 0xA9);

    QCOMPARE(m_cpu.programCounter(), quint16(0x0000));

    clock();                            // C2: Operand from DDR
    verifyReadCycle(0x0000);

    QCOMPARE(m_cpu.accumulator(), quint8(0x2A));
    QCOMPARE(m_cpu.programCounter(), quint16(0x0001));
}

void MOS6510TestPort::testIndexedIndirectPointerWrap()
{
    setupCpu();

    //
    // Configure DDR $00 = $12.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x12);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x00);

    clock();
    verifyWriteCycle(0x0000);

    //
    // LDA ($FE,X), X=$01
    //
    // $FE + $01 = $FF
    //
    // Pointer low  = [$00FF] = $34
    // Pointer high = [$0000] = $12
    //
    // Effective address = $1234.
    //
    m_memory.writeRAM(0x2000, 0xA1);    // LDA ($FE,X)
    m_memory.writeRAM(0x2001, 0xFE);
    m_memory.writeRAM(0x2002, 0xEA);

    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x1234, 0x37);

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setXRegister(0x01);

    clock();                            // C1: Opcode fetch
    verifyRead(0x2000, 0xA1);

    clock();                            // C2: Zero-page operand
    verifyRead(0x2001, 0xFE);

    clock();                            // C3: Indexed dummy read
    verifyRead(0x00FE, m_memory.readRAM(0x00FE));

    clock();                            // C4: Pointer low
    verifyRead(0x00FF, 0x34);

    clock();                            // C5: Pointer high from DDR
    verifyReadCycle(0x0000);

    clock();                            // C6: Data
    verifyRead(0x1234, 0x37);

    QCOMPARE(m_cpu.accumulator(), quint8(0x37));

    clock();                            // Next opcode
    verifyRead(0x2002, 0xEA);
}

void MOS6510TestPort::testIndirectIndexedPointerWrap()
{
    setupCpu();

    //
    // Configure DDR $00 = $12.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x12);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x00);

    clock();
    verifyWriteCycle(0x0000);

    //
    // LDA ($FF),Y with Y=$00.
    //
    // Pointer low  = [$00FF] = $34
    // Pointer high = [$0000] = $12
    //
    // Effective address = $1234.
    //
    m_memory.writeRAM(0x2000, 0xB1);    // LDA ($FF),Y
    m_memory.writeRAM(0x2001, 0xFF);
    m_memory.writeRAM(0x2002, 0xEA);

    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x1234, 0x37);

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setYRegister(0x00);

    clock();                            // C1: Opcode fetch
    verifyRead(0x2000, 0xB1);

    clock();                            // C2: Zero-page pointer
    verifyRead(0x2001, 0xFF);

    clock();                            // C3: Pointer low
    verifyRead(0x00FF, 0x34);

    clock();                            // C4: Pointer high from DDR
    verifyReadCycle(0x0000);

    clock();                            // C5: Data
    verifyRead(0x1234, 0x37);

    QCOMPARE(m_cpu.accumulator(), quint8(0x37));

    clock();                            // Next opcode
    verifyRead(0x2002, 0xEA);
}

void MOS6510TestPort::testStoreAccumulatorToDataDirectionRegister()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x11);

    m_memory.writeRAM(0x0000, 0xA5);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);
    m_memory.writeRAM(0x1002, 0xA5);    // LDA $00
    m_memory.writeRAM(0x1003, 0x00);
    m_memory.writeRAM(0x1004, 0xEA);

    clock();                            // STA C1
    verifyRead(0x1000, 0x85);

    clock();                            // STA C2
    verifyRead(0x1001, 0x00);

    clock();                            // STA C3
    verifyWriteCycle(0x0000);

    QCOMPARE(m_memory.readRAM(0x0000), quint8(0x00));

    clock();                            // LDA C1
    verifyRead(0x1002, 0xA5);

    clock();                            // LDA C2
    verifyRead(0x1003, 0x00);

    clock();                            // LDA C3
    verifyReadCycle(0x0000);

    QCOMPARE(m_cpu.accumulator(), quint8(0x11));

    clock();
    verifyRead(0x1004, 0xEA);
}

void MOS6510TestPort::testStoreXToDataDirectionRegister()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setXRegister(0x22);

    m_memory.writeRAM(0x0000, 0xA5);

    m_memory.writeRAM(0x1000, 0x86);    // STX $00
    m_memory.writeRAM(0x1001, 0x00);
    m_memory.writeRAM(0x1002, 0xA5);    // LDA $00
    m_memory.writeRAM(0x1003, 0x00);
    m_memory.writeRAM(0x1004, 0xEA);

    clock();                            // STX C1
    verifyRead(0x1000, 0x86);

    clock();                            // STX C2
    verifyRead(0x1001, 0x00);

    clock();                            // STX C3
    verifyWriteCycle(0x0000);

    QCOMPARE(m_memory.readRAM(0x0000), quint8(0x00));

    clock();                            // LDA C1
    verifyRead(0x1002, 0xA5);

    clock();                            // LDA C2
    verifyRead(0x1003, 0x00);

    clock();                            // LDA C3
    verifyReadCycle(0x0000);

    QCOMPARE(m_cpu.accumulator(), quint8(0x22));

    clock();
    verifyRead(0x1004, 0xEA);
}

void MOS6510TestPort::testStoreYToDataDirectionRegister()
{
    setupCpu();

    m_cpu.setProgramCounter(0x1000);
    m_cpu.setYRegister(0x33);

    m_memory.writeRAM(0x0000, 0xA5);

    m_memory.writeRAM(0x1000, 0x84);    // STY $00
    m_memory.writeRAM(0x1001, 0x00);
    m_memory.writeRAM(0x1002, 0xA5);    // LDA $00
    m_memory.writeRAM(0x1003, 0x00);
    m_memory.writeRAM(0x1004, 0xEA);

    clock();                            // STY C1
    verifyRead(0x1000, 0x84);

    clock();                            // STY C2
    verifyRead(0x1001, 0x00);

    clock();                            // STY C3
    verifyWriteCycle(0x0000);

    QCOMPARE(m_memory.readRAM(0x0000), quint8(0x00));

    clock();                            // LDA C1
    verifyRead(0x1002, 0xA5);

    clock();                            // LDA C2
    verifyRead(0x1003, 0x00);

    clock();                            // LDA C3
    verifyReadCycle(0x0000);

    QCOMPARE(m_cpu.accumulator(), quint8(0x33));

    clock();
    verifyRead(0x1004, 0xEA);
}

void MOS6510TestPort::testIndexedIndirectStorePointerWrap()
{
    setupCpu();

    //
    // Configure DDR $00 = $12.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x12);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x00);

    clock();
    verifyWriteCycle(0x0000);

    //
    // STA ($FE,X), X=$01
    //
    // $FE + $01 = $FF
    //
    // Pointer low  = [$00FF] = $34
    // Pointer high = [$0000] = $12
    //
    // Effective address = $1234.
    //
    m_memory.writeRAM(0x2000, 0x81);    // STA ($FE,X)
    m_memory.writeRAM(0x2001, 0xFE);
    m_memory.writeRAM(0x2002, 0xEA);

    m_memory.writeRAM(0x00FE, 0x55);
    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x1234, 0x00);

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setAccumulator(0x11);
    m_cpu.setXRegister(0x01);

    clock();                            // C1: Opcode fetch
    verifyRead(0x2000, 0x81);

    clock();                            // C2: Zero-page operand
    verifyRead(0x2001, 0xFE);

    clock();                            // C3: Indexed dummy read
    verifyRead(0x00FE, 0x55);

    clock();                            // C4: Pointer low
    verifyRead(0x00FF, 0x34);

    clock();                            // C5: Pointer high from DDR
    verifyReadCycle(0x0000);

    clock();                            // C6: Store
    verifyWrite(0x1234, 0x11);

    QCOMPARE(m_memory.readRAM(0x1234), quint8(0x11));

    clock();                            // Next opcode
    verifyRead(0x2002, 0xEA);
}

void MOS6510TestPort::testIndirectIndexedStorePointerWrap()
{
    setupCpu();

    //
    // Configure DDR $00 = $12.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x12);

    m_memory.writeRAM(0x1000, 0x85);    // STA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();
    verifyRead(0x1000, 0x85);

    clock();
    verifyRead(0x1001, 0x00);

    clock();
    verifyWriteCycle(0x0000);

    //
    // STA ($FF),Y with Y=$01.
    //
    // Pointer low  = [$00FF] = $34
    // Pointer high = [$0000] = $12
    //
    // Base address      = $1234
    // Effective address = $1235
    //
    m_memory.writeRAM(0x2000, 0x91);    // STA ($FF),Y
    m_memory.writeRAM(0x2001, 0xFF);
    m_memory.writeRAM(0x2002, 0xEA);

    m_memory.writeRAM(0x00FF, 0x34);
    m_memory.writeRAM(0x1235, 0x00);

    m_cpu.setProgramCounter(0x2000);
    m_cpu.setAccumulator(0x11);
    m_cpu.setYRegister(0x01);

    clock();                            // C1: Opcode fetch
    verifyRead(0x2000, 0x91);

    clock();                            // C2: Zero-page pointer
    verifyRead(0x2001, 0xFF);

    clock();                            // C3: Pointer low
    verifyRead(0x00FF, 0x34);

    clock();                            // C4: Pointer high from DDR
    verifyReadCycle(0x0000);

    clock();                            // C5: Dummy read
    verifyRead(0x1235, 0x00);

    clock();                            // C6: Store
    verifyWrite(0x1235, 0x11);

    QCOMPARE(m_memory.readRAM(0x1235), quint8(0x11));

    clock();                            // Next opcode
    verifyRead(0x2002, 0xEA);
}

void MOS6510TestPort::testDataDirectionRegisterReadInternalValueExternalRamValue()
{
    setupCpu();

    //
    // Configure the internal DDR with $2A.
    //
    setDataDirectionRegister(0x2A);

    //
    // Physical RAM underneath the processor port contains a different value.
    //
    m_memory.writeRAM(0x0000, 0xC3);

    m_cpu.setProgramCounter(0x1000);

    m_memory.writeRAM(0x1000, 0xA5);    // LDA $00
    m_memory.writeRAM(0x1001, 0x00);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0xA5);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x00);

    clock();                            // C3: Read DDR
    verifyReadCycle(0x0000);

    //
    // The CPU sees the internal DDR value.
    //
    QCOMPARE(m_cpu.accumulator(), quint8(0x2A));

    //
    // The external data bus sees physical RAM underneath $0000.
    //
    QCOMPARE(m_bus.dataBusValue(), quint8(0xC3));
    QCOMPARE(m_bus.lastAccessValue(), quint8(0xC3));
    QVERIFY(!m_bus.cpuDrivesDataBus());
}

void MOS6510TestPort::testDataRegisterReadInternalValueExternalRamValue()
{
    setupCpu();

    //
    // Configure P0-P5 as outputs.
    //
    setDataDirectionRegister(0x3F);

    //
    // Write $15 to the internal processor port data register.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x15);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    //
    // Physical RAM underneath the processor port contains a different value.
    //
    m_memory.writeRAM(0x0001, 0xA6);

    //
    // Read $0001.
    //
    m_cpu.setProgramCounter(0x1100);

    m_memory.writeRAM(0x1100, 0xA5);    // LDA $01
    m_memory.writeRAM(0x1101, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1100, 0xA5);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1101, 0x01);

    clock();                            // C3: Read data register
    verifyReadCycle(0x0001);

    //
    // The CPU sees the internal processor port value.
    //
    QCOMPARE(m_cpu.accumulator(), quint8(0x15));

    //
    // The external data bus sees physical RAM underneath $0001.
    //
    QCOMPARE(m_bus.dataBusValue(), quint8(0xA6));
    QCOMPARE(m_bus.lastAccessValue(), quint8(0xA6));
    QVERIFY(!m_bus.cpuDrivesDataBus());
}

void MOS6510TestPort::testDataRegisterBit6FalloffAfterOutputToInput()
{
    setupCpu();

    //
    // Configure bit 6 as output.
    //
    setDataDirectionRegister(0x40);

    //
    // Write 1 to bit 6 of the processor-port data register.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x40);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    //
    // Change bit 6 from output to input.
    //
    setDataDirectionRegister(0x00);

    //
    // Read the processor-port data register immediately.
    //
    m_cpu.setProgramCounter(0x1100);

    m_memory.writeRAM(0x1100, 0xA5);    // LDA $01
    m_memory.writeRAM(0x1101, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1100, 0xA5);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1101, 0x01);

    clock();                            // C3: Read data register
    verifyReadCycle(0x0001);

    //
    // Bit 6 retains its previous high state immediately after
    // changing from output to input.
    //
    QCOMPARE(m_cpu.accumulator() & quint8(0x40), quint8(0x40));
}

void MOS6510TestPort::testDataRegisterBit7FalloffAfterOutputToInput()
{
    setupCpu();

    //
    // Configure bit 7 as output.
    //
    setDataDirectionRegister(0x80);

    //
    // Write 1 to bit 7 of the processor-port data register.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x80);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    //
    // Change bit 7 from output to input.
    //
    setDataDirectionRegister(0x00);

    //
    // Read the processor-port data register immediately.
    //
    m_cpu.setProgramCounter(0x1100);

    m_memory.writeRAM(0x1100, 0xA5);    // LDA $01
    m_memory.writeRAM(0x1101, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1100, 0xA5);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1101, 0x01);

    clock();                            // C3: Read data register
    verifyReadCycle(0x0001);

    //
    // Bit 7 retains its previous high state immediately after
    // changing from output to input.
    //
    QCOMPARE(m_cpu.accumulator() & quint8(0x80), quint8(0x80));
}

void MOS6510TestPort::testDataRegisterBits67LowAfterOutputToInput()
{
    setupCpu();

    //
    // Configure bits 6 and 7 as outputs.
    //
    setDataDirectionRegister(0xC0);

    //
    // Write 0 to bits 6 and 7 of the processor-port data register.
    //
    m_cpu.setProgramCounter(0x1000);
    m_cpu.setAccumulator(0x00);

    m_memory.writeRAM(0x1000, 0x85);    // STA $01
    m_memory.writeRAM(0x1001, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1000, 0x85);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1001, 0x01);

    clock();                            // C3: Write data register
    verifyWriteCycle(0x0001);

    //
    // Change bits 6 and 7 from output to input.
    //
    setDataDirectionRegister(0x00);

    //
    // Read the processor-port data register immediately.
    //
    m_cpu.setProgramCounter(0x1100);

    m_memory.writeRAM(0x1100, 0xA5);    // LDA $01
    m_memory.writeRAM(0x1101, 0x01);

    clock();                            // C1: Fetch opcode
    verifyRead(0x1100, 0xA5);

    clock();                            // C2: Fetch zero-page address
    verifyRead(0x1101, 0x01);

    clock();                            // C3: Read data register
    verifyReadCycle(0x0001);

    //
    // Both floating inputs retain their previous low state.
    //
    QCOMPARE(m_cpu.accumulator() & quint8(0xC0), quint8(0x00));
}
