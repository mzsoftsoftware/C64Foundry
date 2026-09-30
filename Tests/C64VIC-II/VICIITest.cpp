#include "VICIITest.h"

#include <QTest>

#include "C64/C64Timing.h"
#include "C64/Bus/C64Bus.h"
#include "C64/Memory/C64Memory.h"
#include "C64/VIC-II/VIC-II.h"


void VICIITest::testInitialRegisters()
{
    VICII vicII;

    QCOMPARE(vicII.readRegister(0x00), quint8(0x00));
    QCOMPARE(vicII.readRegister(0x20), quint8(0xF0));
}

void VICIITest::testBorderColorRegister()
{
    VICII vicII;

    vicII.writeRegister(0x20, 0x05);

    QCOMPARE(vicII.readRegister(0x20), quint8(0xF5));

    //
    // The border color register stores only the lower four bits.
    // Unused bits read back as one.
    //
    vicII.writeRegister(0x20, 0xA7);

    QCOMPARE(vicII.readRegister(0x20), quint8(0xF7));
}

void VICIITest::testBackgroundColorRegister()
{
    VICII vicII;

    vicII.writeRegister(0x21, 0x03);

    QCOMPARE(vicII.readRegister(0x21), quint8(0xF3));

    //
    // The background color register stores only the lower four bits.
    // Unused bits read back as one.
    //
    vicII.writeRegister(0x21, 0xAC);

    QCOMPARE(vicII.readRegister(0x21), quint8(0xFC));
}

void VICIITest::testColorRegisterBoundaries()
{
    VICII vicII;

    //
    // First color register: $20.
    //
    vicII.writeRegister(0x20, 0x01);
    QCOMPARE(vicII.readRegister(0x20), quint8(0xF1));

    //
    // Last color register: $2E.
    //
    vicII.writeRegister(0x2E, 0x0E);
    QCOMPARE(vicII.readRegister(0x2E), quint8(0xFE));

    //
    // $2F-$3F are unused. Reads return $FF and writes are ignored.
    //
    vicII.writeRegister(0x2F, 0x07);
    QCOMPARE(vicII.readRegister(0x2F), quint8(0xFF));

    //
    // $2F-$3F are unused. Reads return $FF and writes are ignored.
    //
    vicII.writeRegister(0x2F, 0x07);
    vicII.writeRegister(0x3F, 0x08);

    QCOMPARE(vicII.readRegister(0x2F), quint8(0xFF));
    QCOMPARE(vicII.readRegister(0x3F), quint8(0xFF));
}

void VICIITest::testSprite0PositionRegisters()
{
    VICII vicII;

    //
    // Sprite 0 X position: $00.
    //
    vicII.writeRegister(0x00, 0x12);
    QCOMPARE(vicII.readRegister(0x00), quint8(0x12));

    //
    // Sprite 0 Y position: $01.
    //
    vicII.writeRegister(0x01, 0x34);
    QCOMPARE(vicII.readRegister(0x01), quint8(0x34));
}

void VICIITest::testSpriteXMSBRegister()
{
    VICII vicII;

    //
    // Each bit contains the ninth X position bit of one sprite.
    //
    vicII.writeRegister(0x10, 0x81);
    QCOMPARE(vicII.readRegister(0x10), quint8(0x81));

    vicII.writeRegister(0x10, 0x5A);
    QCOMPARE(vicII.readRegister(0x10), quint8(0x5A));
}
void VICIITest::testSpriteEnableRegister()
{
    VICII vicII;

    //
    // Each bit enables one sprite.
    //
    vicII.writeRegister(0x15, 0x81);
    QCOMPARE(vicII.readRegister(0x15), quint8(0x81));

    vicII.writeRegister(0x15, 0x5A);
    QCOMPARE(vicII.readRegister(0x15), quint8(0x5A));
}
void VICIITest::testSpriteYExpansionRegister()
{
    VICII vicII;

    //
    // Each bit enables Y expansion for one sprite.
    //
    vicII.writeRegister(0x17, 0x81);
    QCOMPARE(vicII.readRegister(0x17), quint8(0x81));

    vicII.writeRegister(0x17, 0x5A);
    QCOMPARE(vicII.readRegister(0x17), quint8(0x5A));
}
void VICIITest::testSpriteDataPriorityRegister()
{
    VICII vicII;

    //
    // Each bit controls the data priority of one sprite.
    //
    vicII.writeRegister(0x1B, 0x81);
    QCOMPARE(vicII.readRegister(0x1B), quint8(0x81));

    vicII.writeRegister(0x1B, 0x5A);
    QCOMPARE(vicII.readRegister(0x1B), quint8(0x5A));
}

void VICIITest::testSpriteMulticolorRegister()
{
    VICII vicII;

    //
    // Each bit enables multicolor mode for one sprite.
    //
    vicII.writeRegister(0x1C, 0x81);
    QCOMPARE(vicII.readRegister(0x1C), quint8(0x81));

    vicII.writeRegister(0x1C, 0x5A);
    QCOMPARE(vicII.readRegister(0x1C), quint8(0x5A));
}

void VICIITest::testSpriteXExpansionRegister()
{
    VICII vicII;

    //
    // Each bit enables X expansion for one sprite.
    //
    vicII.writeRegister(0x1D, 0x81);
    QCOMPARE(vicII.readRegister(0x1D), quint8(0x81));

    vicII.writeRegister(0x1D, 0x5A);
    QCOMPARE(vicII.readRegister(0x1D), quint8(0x5A));
}

void VICIITest::testControlRegister1()
{
    VICII vicII;

    //
    // Bits 0-6 contain the VIC-II control state.
    //
    vicII.writeRegister(0x11, 0x55);
    QCOMPARE(vicII.readRegister(0x11), quint8(0x55));

    //
    // Bit 7 selects raster compare bit 8 when written, but reads
    // the current raster counter bit 8 instead.
    //
    vicII.writeRegister(0x11, 0xD5);
    QCOMPARE(vicII.readRegister(0x11), quint8(0x55));
}
void VICIITest::testControlRegister2()
{
    VICII vicII;

    //
    // Bits 0-5 contain the VIC-II control state.
    // Unused bits 6-7 read back as one.
    //
    vicII.writeRegister(0x16, 0x15);
    QCOMPARE(vicII.readRegister(0x16), quint8(0xD5));

    //
    // Only bits 0-5 are writable.
    //
    vicII.writeRegister(0x16, 0xEA);
    QCOMPARE(vicII.readRegister(0x16), quint8(0xEA));
}

void VICIITest::testRasterCounterRegister()
{
    VICII vicII;

    //
    // $12 reads the lower eight bits of the current raster counter.
    //
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));
}




void VICIITest::testClockWithinRasterLine()
{
    VICII vicII;

    //
    // Clocking within the first raster line must not change
    // the raster counter yet.
    //
    vicII.clock();
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));
}
void VICIITest::testRasterLineAdvance()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // The raster counter must remain at line 0 for the first
    // 62 cycles of a PAL raster line.
    //
    for (quint8 cycle = 0; cycle < 62; ++cycle)
        vicII.clock();
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));

    //
    // PAL has 63 cycles per raster line. The 63rd cycle advances
    // the raster counter to line 1.
    //
    vicII.clock();
    QCOMPARE(vicII.readRegister(0x12), quint8(0x01));
}
void VICIITest::testRasterFrameWrap()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Advance through one complete PAL frame.
    //
    for (quint64 cycle = 0; cycle < C64::PALTiming.cyclesPerFrame; ++cycle)
        vicII.clock();
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));
}
void VICIITest::testRasterCounterBit8()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Advance to raster line 256.
    //
    for (quint64 cycle = 0; cycle < 256 * C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }

    //
    // $12 contains raster counter bits 0-7.
    // Bit 7 of $11 contains raster counter bit 8.
    //
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));
    QCOMPARE(vicII.readRegister(0x11) & 0x80, quint8(0x80));
}
void VICIITest::testRasterCompareRegister()
{
    VICII vicII;

    //
    // Writing $12 changes the raster compare value, but must not
    // change the current raster counter returned when reading $12.
    //
    vicII.writeRegister(0x12, 0x80);
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));
}
void VICIITest::testRasterIRQStatus()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Set raster compare to line 1.
    //
    vicII.writeRegister(0x12, 0x01);

    //
    // Advance to raster line 1.
    //
    for (quint8 cycle = 0; cycle < C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }

    //
    // $19 bit 0 indicates a raster interrupt.
    //
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x01));
}
void VICIITest::testRasterCompareBit8()
{
    VICII vicII;

    vicII.setTiming(C64::PALTiming);

    //
    // Set raster compare to line 256.
    //
    vicII.writeRegister(0x11, 0x80);
    vicII.writeRegister(0x12, 0x00);

    //
    // Advance to raster line 256.
    //
    for (quint64 cycle = 0;
         cycle < 256 * C64::PALTiming.cyclesPerLine;
         ++cycle)
    {
        vicII.clock();
    }

    //
    // $19 bit 0 indicates a raster interrupt.
    //
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x01));
}
void VICIITest::testRasterIRQAcknowledge()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Set raster compare to line 1.
    //
    vicII.writeRegister(0x12, 0x01);
    for (quint8 cycle = 0; cycle < C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }

    //
    // Raster interrupt is pending.
    //
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x01));

    //
    // Writing one to bit 0 acknowledges the raster interrupt.
    //
    vicII.writeRegister(0x19, 0x01);
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x00));
}

void VICIITest::testInterruptMaskRegister()
{
    VICII vicII;

    //
    // Bits 0-3 enable the four VIC-II interrupt sources.
    //
    vicII.writeRegister(0x1A, 0x05);
    QCOMPARE(vicII.readRegister(0x1A), quint8(0xF5));

    //
    // Bits 4-7 are unused and always read as one.
    //
    vicII.writeRegister(0x1A, 0xFA);
    QCOMPARE(vicII.readRegister(0x1A), quint8(0xFA));
}
void VICIITest::testIRQStatusBit()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Enable raster interrupts.
    //
    vicII.writeRegister(0x1A, 0x01);

    //
    // Set raster compare to line 1.
    //
    vicII.writeRegister(0x12, 0x01);

    //
    // Advance to raster line 1.
    //
    for (quint8 cycle = 0;
         cycle < C64::PALTiming.cyclesPerLine;
         ++cycle)
    {
        vicII.clock();
    }

    //
    // Bit 0 indicates the pending raster interrupt.
    // Bit 7 indicates an active IRQ.
    //
    QCOMPARE(vicII.readRegister(0x19) & 0x81, quint8(0x81));
}
void VICIITest::testMaskedRasterIRQ()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Raster interrupts remain disabled.
    //
    vicII.writeRegister(0x1A, 0x00);

    //
    // Set raster compare to line 1.
    //
    vicII.writeRegister(0x12, 0x01);

    //
    // Advance to raster line 1.
    //
    for (quint8 cycle = 0; cycle < C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }

    //
    // The raster interrupt is pending, but IRQ is not active
    // because the interrupt source is masked.
    //
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x01));
    QCOMPARE(vicII.readRegister(0x19) & 0x80, quint8(0x00));
}
void VICIITest::testEnablePendingRasterIRQ()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Raster interrupts are initially disabled.
    //
    vicII.writeRegister(0x1A, 0x00);

    //
    // Set raster compare to line 1.
    //
    vicII.writeRegister(0x12, 0x01);

    //
    // Advance to raster line 1.
    //
    for (quint8 cycle = 0; cycle < C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }

    //
    // The raster interrupt is pending, but IRQ is still inactive.
    //
    QCOMPARE(vicII.readRegister(0x19) & 0x81, quint8(0x01));

    //
    // Enabling an already pending interrupt source must immediately
    // activate IRQ.
    //
    vicII.writeRegister(0x1A, 0x01);
    QCOMPARE(vicII.readRegister(0x19) & 0x81, quint8(0x81));
}
void VICIITest::testIRQLine()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // No interrupt is pending initially.
    //
    QCOMPARE(vicII.irq(), false);

    //
    // Enable raster interrupts and set raster compare to line 1.
    //
    vicII.writeRegister(0x1A, 0x01);
    vicII.writeRegister(0x12, 0x01);

    //
    // Advance to raster line 1.
    //
    for (quint8 cycle = 0; cycle < C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }

    //
    // A pending and enabled raster interrupt asserts IRQ.
    //
    QCOMPARE(vicII.irq(), true);

    //
    // Acknowledge the raster interrupt.
    //
    vicII.writeRegister(0x19, 0x01);
    QCOMPARE(vicII.irq(), false);
}
void VICIITest::testDisablePendingRasterIRQ()
{
    VICII vicII;
    vicII.setTiming(C64::PALTiming);

    //
    // Enable raster interrupts and set raster compare to line 1.
    //
    vicII.writeRegister(0x1A, 0x01);
    vicII.writeRegister(0x12, 0x01);
    for (quint8 cycle = 0; cycle < C64::PALTiming.cyclesPerLine; ++cycle)
    {
        vicII.clock();
    }
    QCOMPARE(vicII.irq(), true);
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x01));

    //
    // Disabling the interrupt source deasserts IRQ, but does not
    // clear the pending interrupt status.
    //
    vicII.writeRegister(0x1A, 0x00);
    QCOMPARE(vicII.irq(), false);
    QCOMPARE(vicII.readRegister(0x19) & 0x01, quint8(0x01));
}

void VICIITest::testMemoryPointerRegister()
{
    VICII vicII;

    //
    // Bits 1-7 contain the VIC-II memory pointer configuration.
    // Unused bit 0 reads back as one.
    //
    vicII.writeRegister(0x18, 0x14);
    QCOMPARE(vicII.readRegister(0x18), quint8(0x15));

    //
    // Bit 0 is not writable and always reads back as one.
    //
    vicII.writeRegister(0x18, 0xFE);
    QCOMPARE(vicII.readRegister(0x18), quint8(0xFF));
}

void VICIITest::testMemoryRead()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // The VIC-II accesses memory through the C64 bus.
    //
    memory.writeRAM(0x0234, 0x42);
    QCOMPARE(vicII.readMemory(0x0234), quint8(0x42));
}

void VICIITest::testVideoMatrixBaseAddress()
{
    VICII vicII;

    //
    // Bits 4-7 of $D018 select the Video Matrix base address
    // within the 16 KiB VIC-II address space.
    //
    vicII.writeRegister(0x18, 0x00);
    QCOMPARE(vicII.videoMatrixBaseAddress(), quint16(0x0000));

    vicII.writeRegister(0x18, 0x10);
    QCOMPARE(vicII.videoMatrixBaseAddress(), quint16(0x0400));

    vicII.writeRegister(0x18, 0x40);
    QCOMPARE(vicII.videoMatrixBaseAddress(), quint16(0x1000));

    vicII.writeRegister(0x18, 0x80);
    QCOMPARE(vicII.videoMatrixBaseAddress(), quint16(0x2000));

    vicII.writeRegister(0x18, 0xF0);
    QCOMPARE(vicII.videoMatrixBaseAddress(), quint16(0x3C00));
}

void VICIITest::testCharacterBaseAddress()
{
    VICII vicII;

    //
    // Bits 1-3 of $D018 select the Character Generator base address
    // within the 16 KiB VIC-II address space.
    //
    vicII.writeRegister(0x18, 0x00);
    QCOMPARE(vicII.characterBaseAddress(), quint16(0x0000));

    vicII.writeRegister(0x18, 0x02);
    QCOMPARE(vicII.characterBaseAddress(), quint16(0x0800));

    vicII.writeRegister(0x18, 0x04);
    QCOMPARE(vicII.characterBaseAddress(), quint16(0x1000));

    vicII.writeRegister(0x18, 0x08);
    QCOMPARE(vicII.characterBaseAddress(), quint16(0x2000));

    vicII.writeRegister(0x18, 0x0E);
    QCOMPARE(vicII.characterBaseAddress(), quint16(0x3800));
}

void VICIITest::testCharacterMemoryRead()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    QByteArray characterROM(4096, 0x00);
    characterROM[0x0103] = 0x42;

    QVERIFY(memory.loadCharacterROM(characterROM));

    //
    // Character Generator base address: $1000.
    //
    vicII.writeRegister(0x18, 0x04);

    //
    // Character $20, row 3:
    //
    // $1000 + ($20 * 8) + 3 = $1103
    //
    QCOMPARE(vicII.readCharacterMemory(0x20, 3), quint8(0x42));
}

void VICIITest::testVideoMatrixMemoryRead()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Video Matrix base address: $0400.
    //
    vicII.writeRegister(0x18, 0x10);

    //
    // Character position 37:
    //
    // $0400 + 37 = $0425
    //
    memory.writeRAM(0x0425, 0x42);

    QCOMPARE(vicII.readVideoMatrixMemory(37), quint8(0x42));
}
