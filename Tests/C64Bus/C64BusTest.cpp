#include "C64BusTest.h"

#include <QTest>

#include "C64/Bus/C64Bus.h"
#include "C64/Memory/C64Memory.h"
#include "C64/VIC-II/VIC-II.h"
#include "C64/CIA/MOS6526.h"


C64BusTest::C64BusTest()
{
}
C64BusTest::~C64BusTest()
{
}

void C64BusTest::testInitialCpuPortLines()
{
    C64Bus bus;

    QCOMPARE(bus.cpuPortLines(), quint8(0x07));

    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::BasicROM);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::IO);
    QCOMPARE(bus.memorySource(0xE000), C64Bus::MemorySource::KernalROM);
}

void C64BusTest::testSetCpuPortLines()
{
    C64Bus bus;

    bus.setCpuPortLines(0x02);
    QCOMPARE(bus.cpuPortLines(), quint8(0x02));

    bus.setCpuPortLines(0x05);
    QCOMPARE(bus.cpuPortLines(), quint8(0x05));

    bus.setCpuPortLines(0xFF);
    QCOMPARE(bus.cpuPortLines(), quint8(0x07));
}

void C64BusTest::testBasicROMMapping()
{
    C64Bus bus;

    //
    // BASIC ROM is visible if LORAM and HIRAM are both high.
    //
    bus.setCpuPortLines(0x03);
    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::BasicROM);
    QCOMPARE(bus.memorySource(0xBFFF), C64Bus::MemorySource::BasicROM);

    //
    // LORAM low disables BASIC ROM.
    //
    bus.setCpuPortLines(0x02);
    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::RAM);
    QCOMPARE(bus.memorySource(0xBFFF), C64Bus::MemorySource::RAM);

    //
    // HIRAM low disables BASIC ROM.
    //
    bus.setCpuPortLines(0x01);
    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::RAM);

    //
    // Both low disable BASIC ROM.
    //
    bus.setCpuPortLines(0x00);
    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::RAM);

    //
    // CHAREN does not affect BASIC ROM mapping.
    //
    bus.setCpuPortLines(0x07);
    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::BasicROM);

    bus.setCpuPortLines(0x03);
    QCOMPARE(bus.memorySource(0xA000), C64Bus::MemorySource::BasicROM);
}

void C64BusTest::testKernalROMMapping()
{
    C64Bus bus;

    //
    // KERNAL ROM is visible if HIRAM is high.
    //
    bus.setCpuPortLines(0x02);
    QCOMPARE(bus.memorySource(0xE000), C64Bus::MemorySource::KernalROM);
    QCOMPARE(bus.memorySource(0xFFFF), C64Bus::MemorySource::KernalROM);

    //
    // LORAM does not affect KERNAL ROM mapping.
    //
    bus.setCpuPortLines(0x03);
    QCOMPARE(bus.memorySource(0xE000), C64Bus::MemorySource::KernalROM);
    QCOMPARE(bus.memorySource(0xFFFF), C64Bus::MemorySource::KernalROM);

    //
    // CHAREN does not affect KERNAL ROM mapping.
    //
    bus.setCpuPortLines(0x06);
    QCOMPARE(bus.memorySource(0xE000), C64Bus::MemorySource::KernalROM);
    QCOMPARE(bus.memorySource(0xFFFF), C64Bus::MemorySource::KernalROM);

    //
    // HIRAM low disables KERNAL ROM.
    //
    bus.setCpuPortLines(0x05);
    QCOMPARE(bus.memorySource(0xE000), C64Bus::MemorySource::RAM);
    QCOMPARE(bus.memorySource(0xFFFF), C64Bus::MemorySource::RAM);

    bus.setCpuPortLines(0x00);
    QCOMPARE(bus.memorySource(0xE000), C64Bus::MemorySource::RAM);
}

void C64BusTest::testCharacterROMAndIOMapping()
{
    C64Bus bus;

    //
    // LORAM=0, HIRAM=0: RAM is visible independently of CHAREN.
    //
    bus.setCpuPortLines(0x00);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::RAM);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::RAM);

    bus.setCpuPortLines(0x04);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::RAM);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::RAM);

    //
    // At least one of LORAM/HIRAM is high and CHAREN is low:
    // Character ROM is visible.
    //
    bus.setCpuPortLines(0x01);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::CharacterROM);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::CharacterROM);

    bus.setCpuPortLines(0x02);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::CharacterROM);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::CharacterROM);

    bus.setCpuPortLines(0x03);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::CharacterROM);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::CharacterROM);

    //
    // At least one of LORAM/HIRAM is high and CHAREN is high:
    // I/O is visible.
    //
    bus.setCpuPortLines(0x05);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::IO);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::IO);

    bus.setCpuPortLines(0x06);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::IO);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::IO);

    bus.setCpuPortLines(0x07);
    QCOMPARE(bus.memorySource(0xD000), C64Bus::MemorySource::IO);
    QCOMPARE(bus.memorySource(0xDFFF), C64Bus::MemorySource::IO);
}

void C64BusTest::testReadMemoryMapping()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    QByteArray basicROM(8192, 0x00);
    QByteArray kernalROM(8192, 0x00);
    QByteArray characterROM(4096, 0x00);

    basicROM[0x0000] = static_cast<char>(0x11);
    kernalROM[0x0000] = static_cast<char>(0x22);
    characterROM[0x0000] = static_cast<char>(0x33);
    QVERIFY(memory.loadBasicROM(basicROM));
    QVERIFY(memory.loadKernalROM(kernalROM));
    QVERIFY(memory.loadCharacterROM(characterROM));

    memory.writeRAM(0x1000, 0x44);
    memory.writeRAM(0xA000, 0x55);
    memory.writeRAM(0xD000, 0x66);
    memory.writeRAM(0xE000, 0x77);

    //
    // Normal RAM.
    //
    QCOMPARE(bus.read(0x1000), static_cast<quint8>(0x44));

    //
    // All ROMs visible.
    //
    bus.setCpuPortLines(0x07);

    QCOMPARE(bus.read(0xA000), static_cast<quint8>(0x11));
    QCOMPARE(bus.read(0xE000), static_cast<quint8>(0x22));

    //
    // Character ROM visible.
    //
    bus.setCpuPortLines(0x03);

    QCOMPARE(bus.read(0xD000), static_cast<quint8>(0x33));

    //
    // RAM below the ROM areas.
    //
    bus.setCpuPortLines(0x00);

    QCOMPARE(bus.read(0xA000), static_cast<quint8>(0x55));
    QCOMPARE(bus.read(0xD000), static_cast<quint8>(0x66));
    QCOMPARE(bus.read(0xE000), static_cast<quint8>(0x77));
}

void C64BusTest::testWriteRAMBelowROM()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    //
    // Enable BASIC and KERNAL ROM.
    //
    bus.setCpuPortLines(0x07);

    //
    // Writes still go to the RAM below the ROMs.
    //
    bus.write(0xA000, 0x11);
    bus.write(0xE000, 0x22);
    QCOMPARE(memory.readRAM(0xA000), static_cast<quint8>(0x11));
    QCOMPARE(memory.readRAM(0xE000), static_cast<quint8>(0x22));

    //
    // Enable Character ROM.
    //
    bus.setCpuPortLines(0x03);
    bus.write(0xD000, 0x33);
    QCOMPARE(memory.readRAM(0xD000), static_cast<quint8>(0x33));
}

void C64BusTest::testColorRAMMapping()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    //
    // I/O visible.
    //
    bus.setCpuPortLines(0x07);

    bus.write(0xD800, 0x05);
    bus.write(0xD923, 0x0A);
    bus.write(0xDBFF, 0x0F);

    QCOMPARE(bus.read(0xD800), quint8(0x05));
    QCOMPARE(bus.read(0xD923), quint8(0x0A));
    QCOMPARE(bus.read(0xDBFF), quint8(0x0F));

    //
    // Only the lower four bits are stored.
    //
    bus.write(0xD900, 0xA7);

    QCOMPARE(bus.read(0xD900), quint8(0x07));
}

void C64BusTest::testVICIIRegisterMapping()
{
    C64Memory memory;
    VICII vicII;
    C64Bus bus;

    bus.setMemory(&memory);
    bus.setVICII(&vicII);

    //
    // I/O visible.
    //
    bus.setCpuPortLines(0x07);

    //
    // $D020 maps to VIC-II register $20.
    //
    bus.write(0xD020, 0x05);
    QCOMPARE(bus.read(0xD020), quint8(0xF5));

    //
    // VIC-II registers are mirrored every $40 bytes
    // throughout $D000-$D3FF.
    //
    QCOMPARE(bus.read(0xD060), quint8(0xF5));
    QCOMPARE(bus.read(0xD0A0), quint8(0xF5));
    QCOMPARE(bus.read(0xD3E0), quint8(0xF5));

    //
    // Writing through a mirror must access the same register.
    //
    bus.write(0xD060, 0x0A);
    QCOMPARE(bus.read(0xD020), quint8(0xFA));

    //
    // Character ROM visible instead of I/O.
    //
    bus.setCpuPortLines(0x03);
    bus.write(0xD020, 0x07);

    //
    // Writes below Character ROM go to RAM and must not reach the VIC-II.
    //
    bus.setCpuPortLines(0x07);
    QCOMPARE(bus.read(0xD020), quint8(0xFA));
}

void C64BusTest::testVICMemoryRead()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    //
    // The VIC-II reads RAM through its own memory access path.
    //
    memory.writeRAM(0x0234, 0x42);

    QCOMPARE(bus.readVIC(0x0234), quint8(0x42));
}
void C64BusTest::testVICMemoryAddressMask()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    //
    // The VIC-II has a 14-bit address bus.
    // Address bits 14 and 15 must therefore be ignored here.
    //
    memory.writeRAM(0x0234, 0x42);
    memory.writeRAM(0x4234, 0x11);
    memory.writeRAM(0x8234, 0x22);
    memory.writeRAM(0xC234, 0x33);

    QCOMPARE(bus.readVIC(0x0234), quint8(0x42));
    QCOMPARE(bus.readVIC(0x4234), quint8(0x42));
    QCOMPARE(bus.readVIC(0x8234), quint8(0x42));
    QCOMPARE(bus.readVIC(0xC234), quint8(0x42));
}
void C64BusTest::testVICCharacterROM()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    QByteArray characterROM(4096, 0x00);
    characterROM[0x0000] = 0x11;
    characterROM[0x0234] = 0x42;
    characterROM[0x0FFF] = 0x33;

    QVERIFY(memory.loadCharacterROM(characterROM));

    //
    // In VIC-II bank 0, $1000-$1FFF is mapped to the
    // Character ROM instead of the underlying RAM.
    //
    memory.writeRAM(0x1000, 0xAA);
    memory.writeRAM(0x1234, 0xBB);
    memory.writeRAM(0x1FFF, 0xCC);

    QCOMPARE(bus.readVIC(0x1000), quint8(0x11));
    QCOMPARE(bus.readVIC(0x1234), quint8(0x42));
    QCOMPARE(bus.readVIC(0x1FFF), quint8(0x33));
}
void C64BusTest::testVICCharacterROMBoundaries()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    QByteArray characterROM(4096, 0x00);
    characterROM[0x0000] = 0x11;
    characterROM[0x0FFF] = 0x22;

    QVERIFY(memory.loadCharacterROM(characterROM));

    //
    // RAM immediately outside the Character ROM area must remain visible.
    //
    memory.writeRAM(0x0FFF, 0x33);
    memory.writeRAM(0x1000, 0x44);
    memory.writeRAM(0x1FFF, 0x55);
    memory.writeRAM(0x2000, 0x66);

    QCOMPARE(bus.readVIC(0x0FFF), quint8(0x33));
    QCOMPARE(bus.readVIC(0x1000), quint8(0x11));
    QCOMPARE(bus.readVIC(0x1FFF), quint8(0x22));
    QCOMPARE(bus.readVIC(0x2000), quint8(0x66));
}

void C64BusTest::testAEC()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    //
    // AEC is high by default.
    //
    QVERIFY(bus.aec());

    //
    // During a normal CPU write cycle, the CPU drives
    // the data bus.
    //
    bus.write(0x1000, 0x42);

    QVERIFY(bus.cpuDrivesDataBus());

    //
    // AEC low disconnects the CPU from the system bus.
    //
    bus.clock();
    bus.setAEC(false);
    bus.write(0x1000, 0x55);

    QVERIFY(!bus.cpuDrivesDataBus());

    //
    // A CPU write while AEC is low must not reach memory.
    //
    memory.writeRAM(0x1000, 0x42);

    bus.clock();
    bus.setAEC(false);
    bus.write(0x1000, 0x55);

    QCOMPARE(memory.readRAM(0x1000), quint8(0x42));
    QVERIFY(!bus.cpuDrivesDataBus());

    //
    // AEC low disconnects the CPU from the system bus.
    // A CPU read must therefore not become a system-bus access
    // and must not change the current data-bus value.
    //
    bus.clock();
    bus.setDataBusValue(0xA5);
    bus.setAEC(false);

    const quint8 value = bus.read(0x1000);

    QCOMPARE(bus.accessCount(), quint8(0));
    QCOMPARE(value, quint8(0xA5));
    QCOMPARE(bus.dataBusValue(), quint8(0xA5));
    QVERIFY(!bus.cpuDrivesDataBus());

    //
    // A CPU port write while AEC is low must not
    // reach the system bus or RAM.
    //
    memory.writeRAM(0x0000, 0x42);

    bus.clock();
    bus.setDataBusValue(0x55);
    bus.setAEC(false);

    bus.writeCycle(0x0000);

    QCOMPARE(bus.accessCount(), quint8(0));
    QCOMPARE(memory.readRAM(0x0000), quint8(0x42));
    QVERIFY(!bus.cpuDrivesDataBus());

    //
    // A CPU port read while AEC is low must not
    // become a system-bus access.
    //
    bus.clock();
    bus.setDataBusValue(0xA5);
    bus.setAEC(false);

    bus.readCycle(0x0000);

    QCOMPARE(bus.accessCount(), quint8(0));
    QCOMPARE(bus.dataBusValue(), quint8(0xA5));
    QVERIFY(!bus.cpuDrivesDataBus());
}

void C64BusTest::testVICReadAccess()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    memory.writeRAM(0x0234, 0x42);

    QCOMPARE(bus.readVIC(0x0234), quint8(0x42));

    QCOMPARE(bus.accessCount(), quint8(1));
    QCOMPARE(bus.lastAccessType(), C64Bus::AccessType::Read);
    QCOMPARE(bus.lastAccessSource(), C64Bus::AccessSource::VICII);
    QCOMPARE(bus.lastAccessAddress(), quint16(0x0234));
    QCOMPARE(bus.lastAccessValue(), quint8(0x42));
}
void C64BusTest::testVICCharacterROMReadAccess()
{
    C64Memory memory;
    C64Bus bus;

    bus.setMemory(&memory);

    QByteArray characterROM(4096, 0x00);
    characterROM[0x0234] = 0x42;

    QVERIFY(memory.loadCharacterROM(characterROM));

    QCOMPARE(bus.readVIC(0x1234), quint8(0x42));

    QCOMPARE(bus.accessCount(), quint8(1));
    QCOMPARE(bus.lastAccessType(), C64Bus::AccessType::Read);
    QCOMPARE(bus.lastAccessSource(), C64Bus::AccessSource::VICII);
    QCOMPARE(bus.lastAccessAddress(), quint16(0x1234));
    QCOMPARE(bus.lastAccessValue(), quint8(0x42));
}

void C64BusTest::testCIA1RegisterMapping()
{
    C64Memory memory;
    MOS6526 cia1;
    C64Bus bus;

    bus.setMemory(&memory);
    bus.setCIA1(&cia1);

    //
    // I/O visible.
    //
    bus.setCpuPortLines(0x07);

    //
    // Configure all Port A pins as outputs.
    //
    bus.write(0xDC02, 0xFF);

    //
    // $DC00 maps to CIA 1 register $00.
    //
    bus.write(0xDC00, 0x5A);
    QCOMPARE(bus.read(0xDC00), quint8(0x5A));

    //
    // CIA registers are mirrored every $10 bytes
    // throughout $DC00-$DCFF.
    //
    QCOMPARE(bus.read(0xDC10), quint8(0x5A));
    QCOMPARE(bus.read(0xDC80), quint8(0x5A));
    QCOMPARE(bus.read(0xDCF0), quint8(0x5A));
}
void C64BusTest::testCIA2RegisterMapping()
{
    C64Memory memory;
    MOS6526 cia2;
    C64Bus bus;

    bus.setMemory(&memory);
    bus.setCIA2(&cia2);

    //
    // I/O visible.
    //
    bus.setCpuPortLines(0x07);

    //
    // Configure all Port A pins as outputs.
    //
    bus.write(0xDD02, 0xFF);

    //
    // $DD00 maps to CIA 2 register $00.
    //
    bus.write(0xDD00, 0x5A);
    QCOMPARE(bus.read(0xDD00), quint8(0x5A));

    //
    // CIA registers are mirrored every $10 bytes
    // throughout $DD00-$DDFF.
    //
    QCOMPARE(bus.read(0xDD10), quint8(0x5A));
    QCOMPARE(bus.read(0xDD80), quint8(0x5A));
    QCOMPARE(bus.read(0xDDF0), quint8(0x5A));
}