#include "C64BusTest.h"

#include <QTest>

#include "C64/Bus/C64Bus.h"
#include "C64/Memory/C64Memory.h"


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
