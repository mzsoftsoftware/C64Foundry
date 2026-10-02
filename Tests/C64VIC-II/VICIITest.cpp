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

void VICIITest::testBadLineRasterAndYScroll()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $30.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    //
    // Raster line $30 is inside the badline range and its
    // lower three bits match YSCROLL.
    //
    QVERIFY(vicII.badLine());

    //
    // Advance to raster line $31.
    //
    for (quint32 cycle = 0;
         cycle < C64::PALTiming.cyclesPerLine;
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(!vicII.badLine());

    //
    // Change YSCROLL to 1. The current raster line $31 now
    // matches YSCROLL and therefore becomes a badline.
    //
    vicII.writeRegister(0x11, 0x11);

    QVERIFY(vicII.badLine());

    //
    // Advance to raster line $32.
    //
    for (quint32 cycle = 0;
         cycle < C64::PALTiming.cyclesPerLine;
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(!vicII.badLine());
}
void VICIITest::testBadLineRasterRange()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $30. This also enables badlines
    // for the current display frame.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(vicII.badLine());

    //
    // Advance to raster line $F0. It is still inside the
    // badline range and matches YSCROLL = 0.
    //
    for (quint32 cycle = 0;
         cycle < ((0xF0 - 0x30) * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(vicII.badLine());

    //
    // Raster line $F8 would match YSCROLL = 0, but lies
    // outside the badline range $30-$F7.
    //
    for (quint32 cycle = 0;
         cycle < (8 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(!vicII.badLine());
}

void VICIITest::testBadLineEnable()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // DEN is disabled.
    //
    vicII.writeRegister(0x11, 0x00);

    //
    // Advance to raster line $30.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(!vicII.badLine());
}
void VICIITest::testBadLineEnableWithDEN()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display. YSCROLL remains zero.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $30.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(vicII.badLine());
}

void VICIITest::testBadLineBA()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $30.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(vicII.badLine());

    //
    // BA is still high before cycle 12.
    //
    while (vicII.rasterCycle() < 11)
        vicII.clock();

    QVERIFY(vicII.ba());

    //
    // BA goes low at cycle 12.
    //
    vicII.clock();
    QVERIFY(!vicII.ba());

    //
    // BA remains low through cycles 13 and 14.
    //
    vicII.clock();
    QVERIFY(!vicII.ba());

    vicII.clock();
    QVERIFY(!vicII.ba());

    //
    // Cycle 15 is the first c-access.
    //
    vicII.clock();
    QVERIFY(!vicII.ba());

    //
    // Advance to cycle 54.
    //
    while (vicII.rasterCycle() < 54)
        vicII.clock();

    QVERIFY(!vicII.ba());

    //
    // BA returns high at cycle 55.
    //
    vicII.clock();
    QVERIFY(vicII.ba());
}
void VICIITest::testNonBadLineBA()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $31, which is not a badline.
    //
    for (quint32 cycle = 0;
         cycle < (0x31 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(!vicII.badLine());

    //
    // BA remains high throughout the complete raster line.
    //
    for (quint8 cycle = 0;
         cycle < C64::PALTiming.cyclesPerLine;
         ++cycle)
    {
        QVERIFY(vicII.ba());
        vicII.clock();
    }
}

void VICIITest::testBadLineAEC()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $30.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(vicII.badLine());

    //
    // AEC remains high through cycle 14.
    //
    while (vicII.rasterCycle() < 14)
        vicII.clock();

    QVERIFY(vicII.aec());

    //
    // AEC goes low at cycle 15.
    //
    vicII.clock();
    QVERIFY(!vicII.aec());

    //
    // AEC remains low through cycle 54.
    //
    while (vicII.rasterCycle() < 54)
        vicII.clock();

    QVERIFY(!vicII.aec());

    //
    // AEC returns high at cycle 55.
    //
    vicII.clock();
    QVERIFY(vicII.aec());
}
void VICIITest::testNonBadLineAEC()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to raster line $31, which is not a badline.
    //
    for (quint32 cycle = 0;
         cycle < (0x31 * C64::PALTiming.cyclesPerLine);
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(!vicII.badLine());

    //
    // AEC remains high throughout the complete raster line.
    //
    for (quint8 cycle = 0;
         cycle < C64::PALTiming.cyclesPerLine;
         ++cycle)
    {
        QVERIFY(vicII.aec());
        vicII.clock();
    }
}

void VICIITest::testBadLineFirstCAccess()
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
    // Enable the display and select YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // First character in the Video Matrix.
    //
    memory.writeRAM(0x0400, 0x42);

    //
    // Advance to raster line $30, cycle 14.
    //
    for (quint32 cycle = 0;
         cycle < (0x30 * C64::PALTiming.cyclesPerLine) + 14;
         ++cycle)
    {
        vicII.clock();
    }

    QVERIFY(vicII.badLine());
    QCOMPARE(vicII.rasterCycle(), quint8(14));

    //
    // No c-access has happened in this bus cycle yet.
    //
    bus.clock();

    //
    // Cycle 15 performs the first c-access.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(15));
    QCOMPARE(bus.accessCount(), quint8(1));
    QCOMPARE(bus.lastAccessType(), C64Bus::AccessType::Read);
    QCOMPARE(bus.lastAccessSource(), C64Bus::AccessSource::VICII);
    QCOMPARE(bus.lastAccessAddress(), quint16(0x0400));
    QCOMPARE(bus.lastAccessValue(), quint8(0x42));

    //
    // The fetched character code is stored in the VIC-II line buffer.
    //
    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));

    //
    // Last character in the Video Matrix.
    //
    memory.writeRAM(0x0427, 0x84);

    //
    // Advance through c-accesses 1 through 38.
    //
    for (quint8 cycle = 16; cycle < 54; ++cycle)
    {
        bus.clock();
        vicII.clock();
    }

    //
    // Cycle 54 performs the last c-access.
    //
    bus.clock();
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(54));
    QCOMPARE(bus.accessCount(), quint8(2));
    QCOMPARE(bus.lastAccessType(), C64Bus::AccessType::Read);
    QCOMPARE(bus.lastAccessSource(), C64Bus::AccessSource::VICII);
    QCOMPARE(bus.lastAccessAddress(), quint16(0x0427));
    QCOMPARE(bus.lastAccessValue(), quint8(0x84));

    //
    // The last fetched character code is stored in the VIC-II line buffer.
    //
    QCOMPARE(vicII.videoMatrixLine(39), quint8(0x84));
}

void VICIITest::testRowCounterIncrement()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to cycle 13 of the first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 13))
    {
        vicII.clock();
    }

    //
    // Cycle 14 of a badline resets RC.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(14));
    QCOMPARE(vicII.rowCounter(), quint8(0));

    //
    // Advance to cycle 57.
    //
    while (vicII.rasterCycle() != 57)
        vicII.clock();

    QCOMPARE(vicII.rowCounter(), quint8(0));

    //
    // Cycle 58 increments RC.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(58));
    QCOMPARE(vicII.rowCounter(), quint8(1));

    //
    // RC is incremented at cycle 58 of each following
    // raster line until it reaches 7.
    //
    for (quint8 expectedRowCounter = 2; expectedRowCounter <= 7; ++expectedRowCounter)
    {
        //
        // Advance to cycle 58 of the next raster line.
        //
        do
        {
            vicII.clock();
        }
        while (vicII.rasterCycle() != 58);

        QCOMPARE(vicII.rowCounter(), expectedRowCounter);
    }

    //
    // RC remains at 7 at cycle 58 of the next raster line.
    //
    do
    {
        vicII.clock();
    }
    while (vicII.rasterCycle() != 58);

    QCOMPARE(vicII.rowCounter(), quint8(7));
}

void VICIITest::testFirstGraphicsAccess()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to cycle 14 of the first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 14))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.rasterLine(), quint16(0x30));
    QCOMPARE(vicII.rasterCycle(), quint16(14));

    //
    // Cycle 14 initializes the video matrix sequencer
    // and starts the display state.
    //
    QCOMPARE(vicII.videoCounter(), quint16(0));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(0));
    QCOMPARE(vicII.displayState(), true);

    //
    // Cycle 15 performs the first c-access, but no
    // graphics access yet. VC and VMLI therefore remain
    // unchanged.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(15));
    QCOMPARE(vicII.videoCounter(), quint16(0));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(0));

    //
    // Cycle 16 performs graphics access #0.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(16));
    QCOMPARE(vicII.videoCounter(), quint16(1));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(1));

    //
    // Advance through graphics access #38 at cycle 54.
    //
    while (vicII.rasterCycle() != 54)
        vicII.clock();

    QCOMPARE(vicII.videoCounter(), quint16(39));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(39));

    //
    // Cycle 55 performs the final graphics access #39.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(55));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(40));
}
void VICIITest::testBadLineStartsDisplayState()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to cycle 13 of the first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 13))
    {
        vicII.clock();
    }

    //
    // The VIC-II is not yet in display state.
    //
    QCOMPARE(vicII.displayState(), false);

    //
    // Cycle 14 of a badline starts the display state.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(14));
    QCOMPARE(vicII.displayState(), true);
}
void VICIITest::testDisplayStateEndsAtRowCounterSeven()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to cycle 14 of the first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 14))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.rowCounter(), quint8(0));
    QCOMPARE(vicII.displayState(), true);

    //
    // Advance until RC has reached 7.
    //
    while (vicII.rowCounter() != 7)
        vicII.clock();

    QCOMPARE(vicII.displayState(), true);

    //
    // Advance to cycle 57 of the current raster line.
    //
    while (vicII.rasterCycle() != 57)
        vicII.clock();

    QCOMPARE(vicII.rowCounter(), quint8(7));
    QCOMPARE(vicII.displayState(), true);

    //
    // At cycle 58 with RC = 7, the VIC-II leaves
    // the display state.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(58));
    QCOMPARE(vicII.rowCounter(), quint8(7));
    QCOMPARE(vicII.displayState(), false);
}

void VICIITest::testFirstGraphicsMemoryAccess()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select video matrix at $0400 and character memory at $0000.
    //
    vicII.writeRegister(0x18, 0x10);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Character $42 is the first character in the video matrix.
    //
    memory.writeRAM(0x0400, 0x42);

    //
    // Character $42, row 0 is located at $0210.
    //
    memory.writeRAM(0x0210, 0xA5);

    //
    // Advance through the first c-access at cycle 15.
    // This loads character $42 into video matrix line position 0.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));

    //
    // The graphics data has not been fetched yet.
    //
    QCOMPARE(vicII.graphicsData(), quint8(0x00));

    //
    // Cycle 16 performs the first graphics access.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(16));

    //
    // Character $42 with RC = 0 addresses $0210.
    //
    QCOMPARE(vicII.graphicsData(), quint8(0xA5));
}
void VICIITest::testGraphicsMemoryAccessSequence()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select video matrix at $0400 and character memory at $0000.
    //
    vicII.writeRegister(0x18, 0x10);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Put two different characters into the first two
    // positions of the video matrix.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeRAM(0x0401, 0x43);

    //
    // Character $42, row 0 is located at $0210.
    // Character $43, row 0 is located at $0218.
    //
    memory.writeRAM(0x0210, 0xA5);
    memory.writeRAM(0x0218, 0x5A);

    //
    // Advance through the first c-access at cycle 15.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    //
    // The first c-access has loaded character $42.
    //
    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));

    //
    // Cycle 16 performs g-access #0 and c-access #1.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(16));
    QCOMPARE(vicII.graphicsData(), quint8(0xA5));
    QCOMPARE(vicII.videoMatrixLine(1), quint8(0x43));

    //
    // Cycle 17 performs g-access #1.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(17));
    QCOMPARE(vicII.graphicsData(), quint8(0x5A));
}
void VICIITest::testVideoCounterBaseUpdate()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to cycle 55 of the first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 55))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.rowCounter(), quint8(0));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoCounterBase(), quint16(0));

    //
    // Advance until RC has reached 7.
    //
    while (vicII.rowCounter() != 7)
        vicII.clock();

    QCOMPARE(vicII.displayState(), true);

    //
    // Advance to cycle 57 of the current raster line.
    //
    while (vicII.rasterCycle() != 57)
        vicII.clock();

    QCOMPARE(vicII.rowCounter(), quint8(7));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoCounterBase(), quint16(0));

    //
    // At cycle 58 with RC = 7, VCBASE is updated
    // from the current video counter.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(58));
    QCOMPARE(vicII.videoCounterBase(), quint16(40));
    QCOMPARE(vicII.displayState(), false);
}
void VICIITest::testVideoCounterReloadFromBase()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Advance to cycle 14 of the first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 14))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.rowCounter(), quint8(0));
    QCOMPARE(vicII.videoCounter(), quint16(0));
    QCOMPARE(vicII.videoCounterBase(), quint16(0));
    QCOMPARE(vicII.displayState(), true);

    //
    // Advance through the character row until RC
    // has reached 7.
    //
    while (vicII.rowCounter() != 7)
        vicII.clock();

    //
    // Advance to cycle 57 of the current raster line.
    //
    while (vicII.rasterCycle() != 57)
        vicII.clock();

    QCOMPARE(vicII.rowCounter(), quint8(7));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoCounterBase(), quint16(0));

    //
    // Cycle 58 copies VC to VCBASE and leaves
    // the display state.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(58));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoCounterBase(), quint16(40));
    QCOMPARE(vicII.displayState(), false);

    //
    // Advance to cycle 13 of the next raster line.
    //
    do
    {
        vicII.clock();
    }
    while (vicII.rasterCycle() != 13);

    QCOMPARE(vicII.videoCounter(), quint16(40));

    //
    // Cycle 14 reloads VC from VCBASE and resets VMLI.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(14));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(0));
}

void VICIITest::testBadLineColorRAMAccess()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select video matrix at $0400.
    //
    vicII.writeRegister(0x18, 0x10);

    //
    // Enable display with YSCROLL = 0.
    // Raster line $30 is therefore a badline.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Put character $42 into the first position
    // of the video matrix.
    //
    memory.writeRAM(0x0400, 0x42);

    //
    // Put color $05 into the corresponding Color RAM
    // position.
    //
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Advance through the first c-access at cycle 15.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    //
    // The c-access loads both the character code and
    // its corresponding color information.
    //
    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));
    QCOMPARE(vicII.colorLine(0), quint8(0x05));
}
void VICIITest::testCAccessUsesVideoCounter()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select video matrix at $0400.
    //
    vicII.writeRegister(0x18, 0x10);

    //
    // Enable display with YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Put different character codes into the first
    // positions of the first and second character rows.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeRAM(0x0428, 0x84);

    //
    // Put corresponding colors into Color RAM.
    //
    memory.writeColorRAM(0x0000, 0x05);
    memory.writeColorRAM(0x0028, 0x0A);

    //
    // Advance through the first c-access of the
    // first badline.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.videoCounter(), quint16(0));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(0));
    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));
    QCOMPARE(vicII.colorLine(0), quint8(0x05));

    //
    // Advance through the first c-access of the next
    // badline. VCBASE has become 40 and cycle 14 has
    // reloaded VC from VCBASE.
    //
    while ((vicII.rasterLine() != 0x38) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.videoCounterBase(), quint16(40));
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(0));

    //
    // The first c-access of the second character row
    // must use VC = 40 for the memory addresses while
    // storing the result at VMLI = 0.
    //
    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x84));
    QCOMPARE(vicII.colorLine(0), quint8(0x0A));
}

void VICIITest::testCAccessVideoCounterSequence()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select video matrix at $0400.
    //
    vicII.writeRegister(0x18, 0x10);

    //
    // Enable display with YSCROLL = 0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Put different values into the first and last
    // positions of the second character row.
    //
    memory.writeRAM(0x0428, 0x42);
    memory.writeRAM(0x044F, 0x84);

    //
    // Put corresponding colors into Color RAM.
    //
    memory.writeColorRAM(0x0028, 0x05);
    memory.writeColorRAM(0x004F, 0x0A);

    //
    // Advance through the first c-access of the
    // second character row.
    //
    while ((vicII.rasterLine() != 0x38) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    //
    // The first c-access uses VC = 40 and stores
    // the result at VMLI = 0.
    //
    QCOMPARE(vicII.videoCounter(), quint16(40));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(0));
    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));
    QCOMPARE(vicII.colorLine(0), quint8(0x05));

    //
    // Advance through the last c-access of the
    // second character row.
    //
    while (vicII.rasterCycle() != 54)
        vicII.clock();

    //
    // At cycle 54, g-access #38 has already advanced
    // VC and VMLI before c-access #39 is performed.
    //
    QCOMPARE(vicII.videoCounter(), quint16(79));
    QCOMPARE(vicII.videoMatrixLineIndex(), quint8(39));

    //
    // The last c-access therefore uses VC = 79 and
    // stores the result at VMLI = 39.
    //
    QCOMPARE(vicII.videoMatrixLine(39), quint8(0x84));
    QCOMPARE(vicII.colorLine(39), quint8(0x0A));
}
