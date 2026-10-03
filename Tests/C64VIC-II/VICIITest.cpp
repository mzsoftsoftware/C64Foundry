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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Clocking within the first raster line must not change
    // the raster counter yet.
    //
    vicII.clock();
    QCOMPARE(vicII.readRegister(0x12), quint8(0x00));
}
void VICIITest::testRasterLineAdvance()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
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

void VICIITest::testFirstGraphicsColor()
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
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Character $42 is the first character in the video matrix.
    //
    memory.writeRAM(0x0400, 0x42);

    //
    // The first character uses color $05.
    //
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Character $42, row 0 is located at $0210.
    //
    memory.writeRAM(0x0210, 0xA5);

    //
    // Advance through the first c-access at cycle 15.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 15))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.videoMatrixLine(0), quint8(0x42));
    QCOMPARE(vicII.colorLine(0), quint8(0x05));

    //
    // Cycle 16 performs graphics access #0.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint16(16));
    QCOMPARE(vicII.graphicsData(), quint8(0xA5));

    //
    // The graphics pipeline must also latch the color
    // belonging to the current character.
    //
    QCOMPARE(vicII.graphicsColor(), quint8(0x05));
}

void VICIITest::testStandardTextGraphicsPixel()
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
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Set background color to $03.
    //
    vicII.writeRegister(0x21, 0x03);

    //
    // Character $42 is the first character in the video matrix
    // and uses color $05.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Character $42, row 0.
    // Bit 7 is set.
    //
    memory.writeRAM(0x0210, 0x80);

    //
    // Advance through graphics access #0 at cycle 16.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.graphicsData(), quint8(0x80));
    QCOMPARE(vicII.graphicsColor(), quint8(0x05));

    //
    // Bit 7 is set, therefore the foreground color
    // comes from Color RAM.
    //
    QCOMPARE(vicII.graphicsPixel(0), quint8(0x05));
}
void VICIITest::testStandardTextGraphicsBackgroundPixel()
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
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Set background color to $03.
    //
    vicII.writeRegister(0x21, 0x03);

    //
    // Character $42 is the first character in the video matrix
    // and uses color $05.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Character $42, row 0.
    // Bit 7 is clear.
    //
    memory.writeRAM(0x0210, 0x00);

    //
    // Advance through graphics access #0 at cycle 16.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    QCOMPARE(vicII.graphicsData(), quint8(0x00));
    QCOMPARE(vicII.graphicsColor(), quint8(0x05));

    //
    // Bit 7 is clear, therefore the pixel uses
    // background color register $D021.
    //
    QCOMPARE(vicII.graphicsPixel(0), quint8(0x03));
}

void VICIITest::testGraphicsPixelPhase()
{
    VICII vicII;

    //
    // A VIC-II clock cycle consists of eight pixel phases.
    //
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(0));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(1));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(2));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(3));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(4));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(5));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(6));

    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(7));

    //
    // After eight pixels, the next VIC-II clock cycle
    // starts again at pixel phase 0.
    //
    vicII.clockGraphicsPixel();
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(0));
}

void VICIITest::testRasterXAdvance()
{
    VICII vicII;

    //
    // A raster line starts at horizontal pixel position 0.
    //
    QCOMPARE(vicII.rasterX(), quint16(0));

    //
    // Each graphics pixel clock advances the horizontal
    // raster position by one pixel.
    //
    vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(1));

    vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(2));

    //
    // One VIC-II clock cycle consists of eight pixels.
    //
    for (quint8 pixel = 2; pixel < 8; ++pixel)
        vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(8));
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(0));
}
void VICIITest::testRasterXLineWrap()
{
    VICII vicII;

    //
    // A PAL raster line consists of 63 VIC-II clock cycles
    // with eight pixels per cycle.
    //
    constexpr quint16 pixelsPerLine = 63 * 8;

    QCOMPARE(vicII.rasterX(), quint16(0));

    //
    // Advance to the last pixel of the raster line.
    //
    for (quint16 pixel = 0; pixel < pixelsPerLine - 1; ++pixel)
        vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(pixelsPerLine - 1));

    //
    // The final pixel advances the horizontal raster position
    // back to the beginning of the next raster line.
    //
    vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(0));
}
void VICIITest::testClockAdvancesGraphicsPixels()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    QCOMPARE(vicII.rasterCycle(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(0));
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(0));

    //
    // One VIC-II clock cycle consists of eight pixel clocks.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(1));
    QCOMPARE(vicII.rasterX(), quint16(8));
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(0));

    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(2));
    QCOMPARE(vicII.rasterX(), quint16(16));
    QCOMPARE(vicII.graphicsPixelPhase(), quint8(0));
}

void VICIITest::testStandardTextGraphicsPixelBuffer()
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
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Set background color to $03.
    //
    vicII.writeRegister(0x21, 0x03);

    //
    // Character $42 is the first character in the video matrix
    // and uses foreground color $05.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Character $42, row 0:
    //
    //     Bit:   7 6 5 4 3 2 1 0
    //     Data:  1 0 1 0 0 1 0 1
    //
    memory.writeRAM(0x0210, 0xA5);

    //
    // Advance through graphics access #0 at cycle 16.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    //
    // The graphics byte fetched during this VIC-II cycle
    // produces eight pixels.
    //
    const quint8 expectedColors[8] =
        {
            0x05,
            0x03,
            0x05,
            0x03,
            0x03,
            0x05,
            0x03,
            0x05
        };

    for (quint8 pixel = 0; pixel < 8; ++pixel)
        QCOMPARE(vicII.graphicsPixel(pixel), expectedColors[pixel]);
}

void VICIITest::testRasterXTracksRasterCycle()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    QCOMPARE(vicII.rasterCycle(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(0));

    for (quint8 cycle = 1; cycle < 63; ++cycle)
    {
        vicII.clock();

        QCOMPARE(vicII.rasterCycle(), cycle);
        QCOMPARE(vicII.rasterX(),
                 static_cast<quint16>(cycle * 8));
    }

    //
    // The 63rd VIC-II cycle completes the PAL raster line.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(0));
}

void VICIITest::testHorizontalBorderTiming()
{
    VICII vicII;

    //
    // Default is 38-column mode (CSEL = 0).
    //
    QCOMPARE(vicII.borderLeft(), quint16(131));
    QCOMPARE(vicII.borderRight(), quint16(435));

    //
    // Select 40-column mode.
    //
    vicII.writeRegister(0x16, 0x08);

    QCOMPARE(vicII.borderLeft(), quint16(124));
    QCOMPARE(vicII.borderRight(), quint16(444));

    //
    // Return to 38-column mode.
    //
    vicII.writeRegister(0x16, 0x00);

    QCOMPARE(vicII.borderLeft(), quint16(131));
    QCOMPARE(vicII.borderRight(), quint16(435));

    //
    // Switch to NTSC timing while 38-column mode is selected.
    //
    vicII.setTiming(C64::NTSCTiming);

    QCOMPARE(vicII.borderLeft(), quint16(139));
    QCOMPARE(vicII.borderRight(), quint16(443));

    //
    // Select 40-column mode.
    //
    vicII.writeRegister(0x16, 0x08);

    QCOMPARE(vicII.borderLeft(), quint16(132));
    QCOMPARE(vicII.borderRight(), quint16(452));

    //
    // Switch back to PAL timing while 40-column mode
    // remains selected.
    //
    vicII.setTiming(C64::PALTiming);

    QCOMPARE(vicII.borderLeft(), quint16(124));
    QCOMPARE(vicII.borderRight(), quint16(444));
}

void VICIITest::testHorizontalBorderRightComparison()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display and select 25-row / 40-column mode.
    //
    vicII.writeRegister(0x11, 0x18);
    vicII.writeRegister(0x16, 0x08);

    QCOMPARE(vicII.borderRight(), quint16(444));
    QVERIFY(vicII.mainBorder());

    //
    // Advance to the top comparison line.
    //
    while (vicII.rasterLine() != vicII.borderTop())
        vicII.clock();

    //
    // Advance to the left border comparison.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    //
    // The left comparison opens the vertical and main borders.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(!vicII.verticalBorder());
    QVERIFY(!vicII.mainBorder());

    //
    // Advance to the pixel immediately before the
    // right border comparison.
    //
    while (vicII.rasterX() < vicII.borderRight() - 1)
        vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(443));
    QVERIFY(!vicII.mainBorder());

    //
    // Pixel X=443 is processed and raster X advances
    // to the right border comparison position.
    //
    vicII.clockGraphicsPixel();

    QCOMPARE(vicII.rasterX(), quint16(444));

    //
    // The comparison at X=444 has not been processed yet.
    //
    QVERIFY(!vicII.mainBorder());

    //
    // Process X=444.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(vicII.mainBorder());
}

void VICIITest::testVerticalBorderTiming()
{
    VICII vicII;

    //
    // Default is 24-row mode (RSEL = 0).
    //
    QCOMPARE(vicII.borderTop(), quint16(55));
    QCOMPARE(vicII.borderBottom(), quint16(247));

    //
    // Select 25-row mode.
    //
    vicII.writeRegister(0x11, 0x08);

    QCOMPARE(vicII.borderTop(), quint16(51));
    QCOMPARE(vicII.borderBottom(), quint16(251));

    //
    // Return to 24-row mode.
    //
    vicII.writeRegister(0x11, 0x00);

    QCOMPARE(vicII.borderTop(), quint16(55));
    QCOMPARE(vicII.borderBottom(), quint16(247));
}
void VICIITest::testVerticalBorderInitialState()
{
    VICII vicII;

    //
    // The vertical border is initially active.
    //
    QVERIFY(vicII.verticalBorder());
}
void VICIITest::testVerticalBorderComparisons()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select 25-row mode, but leave DEN disabled.
    //
    vicII.writeRegister(0x11, 0x08);

    QVERIFY(vicII.verticalBorder());

    //
    // Advance to cycle 62 of the top comparison line.
    // DEN remains disabled, so the earlier left comparison
    // cannot open the vertical border.
    //
    while (vicII.rasterLine() != vicII.borderTop() ||
           vicII.rasterCycle() != 62)
    {
        vicII.clock();
    }

    QVERIFY(vicII.verticalBorder());

    //
    // Enable DEN after the left comparison but before cycle 63.
    //
    vicII.writeRegister(0x11, 0x18);

    QVERIFY(vicII.verticalBorder());

    //
    // Cycle 63 opens the vertical border.
    //
    vicII.clock();

    QVERIFY(!vicII.verticalBorder());
}
void VICIITest::testVerticalBorderRequiresDEN()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Select 25-row mode, but leave DEN disabled.
    //
    vicII.writeRegister(0x11, 0x08);

    QVERIFY(vicII.verticalBorder());

    //
    // Advance to cycle 62 of the top comparison line.
    //
    while (vicII.rasterLine() != vicII.borderTop() ||
           vicII.rasterCycle() != 62)
    {
        vicII.clock();
    }

    QVERIFY(vicII.verticalBorder());

    //
    // Without DEN, the top comparison must not open
    // the vertical border.
    //
    vicII.clock();

    QVERIFY(vicII.verticalBorder());
}

void VICIITest::testMainBorderOpensAtLeft()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display and select 25-row / 40-column mode.
    //
    vicII.writeRegister(0x11, 0x18);
    vicII.writeRegister(0x16, 0x08);

    QVERIFY(vicII.verticalBorder());

    //
    // Advance to the beginning of the top comparison line.
    //
    while (vicII.rasterLine() != vicII.borderTop())
        vicII.clock();

    QVERIFY(vicII.verticalBorder());

    //
    // Advance to the left border comparison.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    QVERIFY(vicII.verticalBorder());

    //
    // The left comparison opens the vertical border.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(!vicII.verticalBorder());

    //
    // Advance to the right border comparison.
    //
    while (vicII.rasterX() != vicII.borderRight())
        vicII.clockGraphicsPixel();

    //
    // The right comparison closes the main border.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(vicII.mainBorder());

    //
    // Advance through the line wrap to the left border comparison.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    QVERIFY(vicII.mainBorder());
    QVERIFY(!vicII.verticalBorder());

    //
    // The left comparison opens the main border
    // while the vertical border is open.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(!vicII.mainBorder());
}
void VICIITest::testMainBorderStaysClosedAtLeftDuringVerticalBorder()
{
    VICII vicII;

    //
    // Select 40-column mode.
    //
    vicII.writeRegister(0x16, 0x08);

    //
    // The vertical and main borders are initially active.
    //
    QVERIFY(vicII.verticalBorder());
    QVERIFY(vicII.mainBorder());

    //
    // Advance to the left border comparison.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    QVERIFY(vicII.verticalBorder());
    QVERIFY(vicII.mainBorder());

    //
    // While the vertical border is active, the left comparison
    // must not open the main border.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(vicII.verticalBorder());
    QVERIFY(vicII.mainBorder());
}

void VICIITest::testVerticalBorderOpensAtLeftComparison()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display and select 25-row / 40-column mode.
    //
    vicII.writeRegister(0x11, 0x18);
    vicII.writeRegister(0x16, 0x08);

    //
    // Advance to the beginning of the top comparison line.
    //
    while (vicII.rasterLine() != vicII.borderTop())
        vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(0));
    QVERIFY(vicII.verticalBorder());

    //
    // Advance only the pixel position to the left border comparison.
    // This deliberately avoids cycle 63.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    QVERIFY(vicII.verticalBorder());

    //
    // The left comparison performs the second vertical-border
    // comparison. With DEN set on the top line, it opens the
    // vertical border.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(!vicII.verticalBorder());
}

void VICIITest::testVerticalBorderClosesAtLeftComparison()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display and select 25-row / 40-column mode.
    //
    vicII.writeRegister(0x11, 0x18);
    vicII.writeRegister(0x16, 0x08);

    //
    // Advance normally until the vertical border has opened.
    //
    while (vicII.verticalBorder())
        vicII.clock();

    QVERIFY(!vicII.verticalBorder());

    //
    // Advance normally to the beginning of the bottom
    // comparison line.
    //
    while (vicII.rasterLine() != vicII.borderBottom())
        vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(0));
    QVERIFY(!vicII.verticalBorder());

    //
    // Advance only the pixel position to the left border
    // comparison. This deliberately avoids cycle 63.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    QVERIFY(!vicII.verticalBorder());

    //
    // The left comparison on the bottom line closes
    // the vertical border.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(vicII.verticalBorder());
}

void VICIITest::testVerticalBorderClosesAtCycle63()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display and select 25-row / 40-column mode.
    //
    vicII.writeRegister(0x11, 0x18);
    vicII.writeRegister(0x16, 0x08);

    //
    // Advance until the vertical border has opened.
    //
    while (vicII.verticalBorder())
        vicII.clock();

    QVERIFY(!vicII.verticalBorder());

    //
    // Before reaching the 25-row bottom comparison line,
    // switch to 24-row mode. Its bottom comparison line
    // (247) will already have passed when we reach line 251.
    //
    while (vicII.rasterLine() < 248)
        vicII.clock();

    vicII.writeRegister(0x11, 0x10);

    QCOMPARE(vicII.borderBottom(), quint16(247));
    QVERIFY(!vicII.verticalBorder());

    //
    // Advance to line 251 and past the left comparison.
    // With RSEL=0, line 251 does not match the current
    // bottom comparison value.
    //
    while (vicII.rasterLine() != 251 ||
           vicII.rasterX() <= vicII.borderLeft())
    {
        vicII.clock();
    }

    QVERIFY(!vicII.verticalBorder());

    //
    // Switch back to 25-row mode after the left comparison.
    // The bottom comparison is now the current raster line.
    //
    vicII.writeRegister(0x11, 0x18);

    QCOMPARE(vicII.borderBottom(), quint16(251));
    QVERIFY(!vicII.verticalBorder());

    //
    // Advance to cycle 62.
    //
    while (vicII.rasterCycle() != 62)
        vicII.clock();

    QVERIFY(!vicII.verticalBorder());

    //
    // Cycle 63 performs the bottom comparison and closes
    // the vertical border.
    //
    vicII.clock();

    QVERIFY(vicII.verticalBorder());
}
void VICIITest::testRasterLineAdvancesWithRasterXWrap()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Advance to cycle 62 of the first raster line.
    //
    while (vicII.rasterCycle() != 62)
        vicII.clock();

    QCOMPARE(vicII.rasterLine(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(496));

    //
    // The final VIC cycle completes the raster line.
    //
    vicII.clock();

    QCOMPARE(vicII.rasterCycle(), quint8(0));
    QCOMPARE(vicII.rasterX(), quint16(0));
    QCOMPARE(vicII.rasterLine(), quint8(1));
}

void VICIITest::testBorderPixelUsesBorderColor()
{
    VICII vicII;

    //
    // Set the border color.
    //
    vicII.writeRegister(0x20, 0x06);

    //
    // The main border is initially active.
    //
    QVERIFY(vicII.mainBorder());

    //
    // Generate one pixel while the main border is active.
    //
    vicII.clockGraphicsPixel();

    //
    // The final output pixel uses the border color from $D020.
    //
    QCOMPARE(vicII.outputPixel(), quint8(0x06));
}
void VICIITest::testGraphicsPixelIgnoresBorderColor()
{
    VICII vicII;

    //
    // Set different border and background colors.
    //
    vicII.writeRegister(0x20, 0x06);
    vicII.writeRegister(0x21, 0x03);

    //
    // The graphics shift register is initially zero,
    // therefore a normal background pixel is generated.
    //
    vicII.clockGraphicsPixel();

    //
    // The graphics pixel is independent of the border
    // and uses the background color from $D021.
    //
    QCOMPARE(vicII.graphicsPixel(0), quint8(0x03));
}

void VICIITest::testOutputPixelUsesGraphicsPixelOutsideBorder()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display and select 25-row / 40-column mode.
    //
    vicII.writeRegister(0x11, 0x18);
    vicII.writeRegister(0x16, 0x08);

    //
    // Set different border and background colors.
    //
    vicII.writeRegister(0x20, 0x06);
    vicII.writeRegister(0x21, 0x03);

    //
    // Advance to the top comparison line.
    //
    while (vicII.rasterLine() != vicII.borderTop())
        vicII.clock();

    //
    // Advance to the left border comparison.
    //
    while (vicII.rasterX() != vicII.borderLeft())
        vicII.clockGraphicsPixel();

    //
    // The left comparison opens the vertical and main borders.
    //
    vicII.clockGraphicsPixel();

    QVERIFY(!vicII.verticalBorder());
    QVERIFY(!vicII.mainBorder());

    //
    // Generate a graphics pixel while the main border is open.
    // The graphics shift register is zero, so the pixel uses
    // the background color from $D021.
    //
    vicII.clockGraphicsPixel();

    QCOMPARE(vicII.outputPixel(), quint8(0x03));
}
void VICIITest::testGraphicsPipelineContinuesDuringBorder()
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
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Set different border and background colors.
    //
    vicII.writeRegister(0x20, 0x06);
    vicII.writeRegister(0x21, 0x03);

    //
    // Character $42 uses foreground color $05.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Character $42, row 0 has bit 7 set.
    //
    memory.writeRAM(0x0210, 0x80);

    //
    // Advance through graphics access #0 at cycle 16
    // of the first bad line.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    QVERIFY(vicII.mainBorder());

    //
    // The graphics pipeline continues to generate the
    // foreground pixel even while the border is active.
    //
    QCOMPARE(vicII.graphicsPixel(0), quint8(0x05));

    //
    // The final output is nevertheless covered by the border.
    //
    QCOMPARE(vicII.outputPixel(), quint8(0x06));
}

void VICIITest::testIdleStateGraphicsAccess()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Store a recognizable value at the idle-state
    // graphics address.
    //
    memory.writeRAM(0x3FFF, 0xA5);

    //
    // The VIC-II starts in idle state.
    //
    QVERIFY(!vicII.displayState());

    //
    // Advance to the first graphics access at cycle 16.
    //
    while (vicII.rasterCycle() != 16)
        vicII.clock();

    QVERIFY(!vicII.displayState());

    //
    // In idle state, a graphics access reads from $3FFF.
    //
    QCOMPARE(vicII.graphicsData(), quint8(0xA5));
}
void VICIITest::testIdleStateECMGraphicsAccess()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Store different values at the normal and ECM
    // idle-state graphics addresses.
    //
    memory.writeRAM(0x3FFF, 0xA5);
    memory.writeRAM(0x39FF, 0x5A);

    //
    // Enable ECM.
    //
    vicII.writeRegister(0x11, 0x40);

    //
    // The VIC-II starts in idle state.
    //
    QVERIFY(!vicII.displayState());

    //
    // Advance to the first graphics access at cycle 16.
    //
    while (vicII.rasterCycle() != 16)
        vicII.clock();

    QVERIFY(!vicII.displayState());

    //
    // In ECM, an idle-state graphics access reads from $39FF.
    //
    QCOMPARE(vicII.graphicsData(), quint8(0x5A));
}
void VICIITest::testIdleStateStandardTextForegroundColor()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);

    //
    // Enable display with YSCROLL=0 so that raster line $30
    // enters display state.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Give all 40 characters of the first video matrix line
    // a non-zero color.
    //
    for (quint16 position = 0; position < 40; ++position)
    {
        memory.writeColorRAM(position, 0x05);
    }

    //
    // Wait for the first graphics access in display state.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    QVERIFY(vicII.displayState());
    QCOMPARE(vicII.graphicsColor(), quint8(0x05));

    //
    // Wait until RC=7 ends the display state in cycle 58
    // of raster line $37.
    //
    while ((vicII.rasterLine() != 0x37) ||
           (vicII.rasterCycle() != 58))
    {
        vicII.clock();
    }

    QVERIFY(!vicII.displayState());

    //
    // The last display-state graphics access must have left
    // the non-zero character color in the graphics color latch.
    //
    QCOMPARE(vicII.graphicsColor(), quint8(0x05));

    //
    // Change YSCROLL so that raster line $38 does not become
    // the next badline.
    //
    vicII.writeRegister(0x11, 0x11);

    //
    // Wait for the first idle-state graphics access
    // on raster line $38.
    //
    while ((vicII.rasterLine() != 0x38) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    QVERIFY(!vicII.displayState());

    //
    // In idle state, video matrix data is treated as zero.
    // Therefore the standard-text foreground color must be zero.
    //
    QCOMPARE(vicII.graphicsColor(), quint8(0x00));
}

void VICIITest::testStandardTextGraphicsSequence()
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
    // Enable display with YSCROLL=0.
    //
    vicII.writeRegister(0x11, 0x10);

    //
    // Set the background color.
    //
    vicII.writeRegister(0x21, 0x03);

    //
    // Character $42 uses foreground color $05.
    //
    memory.writeRAM(0x0400, 0x42);
    memory.writeColorRAM(0x0000, 0x05);

    //
    // Character $42, row 0 contains the bit pattern
    // 10100101.
    //
    memory.writeRAM(0x0210, 0xA5);

    //
    // Advance through graphics access #0 at cycle 16
    // of the first bad line.
    //
    while ((vicII.rasterLine() != 0x30) ||
           (vicII.rasterCycle() != 16))
    {
        vicII.clock();
    }

    QVERIFY(vicII.displayState());
    QCOMPARE(vicII.graphicsData(), quint8(0xA5));

    //
    // Standard text mode shifts the graphics data from
    // bit 7 to bit 0. Set bits use the character color,
    // clear bits use the background color.
    //
    QCOMPARE(vicII.graphicsPixel(0), quint8(0x05));
    QCOMPARE(vicII.graphicsPixel(1), quint8(0x03));
    QCOMPARE(vicII.graphicsPixel(2), quint8(0x05));
    QCOMPARE(vicII.graphicsPixel(3), quint8(0x03));
    QCOMPARE(vicII.graphicsPixel(4), quint8(0x03));
    QCOMPARE(vicII.graphicsPixel(5), quint8(0x05));
    QCOMPARE(vicII.graphicsPixel(6), quint8(0x03));
    QCOMPARE(vicII.graphicsPixel(7), quint8(0x05));
}

void VICIITest::testFrameBufferStoresOutputPixel()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
    vicII.setTiming(C64::PALTiming);

    //
    // Set the border color for the first frame.
    //
    vicII.writeRegister(0x20, 0x06);

    QCOMPARE(vicII.rasterX(), quint16(0));
    QCOMPARE(vicII.rasterLine(), quint16(0));

    //
    // Generate the first VIC-II cycle of the frame.
    //
    vicII.clock();

    //
    // Pixel (0, 0) must contain the final VIC-II output pixel.
    //
    QCOMPARE(vicII.framePixel(0, 0), quint8(0x06));

    //
    // Complete the first PAL frame.
    //
    for (quint64 cycle = 1;
         cycle < C64::PALTiming.cyclesPerFrame;
         ++cycle)
    {
        vicII.clock();
    }

    //
    // The frame buffer index must wrap at the end of the frame.
    //
    QCOMPARE(vicII.frameBufferIndex(), quint32(0));

    //
    // The completed frame must be available as the ready frame.
    //
    QCOMPARE(vicII.readyFramePixel(0, 0), quint8(0x06));

    //
    // Acquire the completed frame for the video consumer.
    //
    quint8* ptrReadFrame = vicII.acquireReadyFrame();

    QVERIFY(ptrReadFrame != nullptr);
    QCOMPARE(ptrReadFrame[0], quint8(0x06));

    //
    // Use a different border color for the second frame.
    //
    vicII.writeRegister(0x20, 0x0E);

    //
    // Generate the complete second frame.
    //
    for (quint64 cycle = 0;
         cycle < C64::PALTiming.cyclesPerFrame;
         ++cycle)
    {
        vicII.clock();
    }

    //
    // The video consumer must still see the first frame.
    //
    QCOMPARE(ptrReadFrame[0], quint8(0x06));

    //
    // The newly completed frame must be available separately.
    //
    QCOMPARE(vicII.readyFramePixel(0, 0), quint8(0x0E));
}

void VICIITest::testFrameBufferAcquireWithoutReadyFrame()
{
    VICII vicII;

    //
    // No complete frame has been generated yet.
    //
    QCOMPARE(vicII.acquireReadyFrame(), nullptr);
}
void VICIITest::testFrameBufferKeepsLatestReadyFrame()
{
    C64Memory memory;
    C64Bus bus;
    VICII vicII;

    bus.setMemory(&memory);
    vicII.setBus(&bus);
    vicII.setTiming(C64::PALTiming);

    //
    // Generate the first frame using border color $06.
    //
    vicII.writeRegister(0x20, 0x06);

    for (quint64 cycle = 0;
         cycle < C64::PALTiming.cyclesPerFrame;
         ++cycle)
    {
        vicII.clock();
    }

    //
    // Do not acquire the completed frame.
    //

    //
    // Generate a second frame using border color $0E.
    //
    vicII.writeRegister(0x20, 0x0E);

    for (quint64 cycle = 0;
         cycle < C64::PALTiming.cyclesPerFrame;
         ++cycle)
    {
        vicII.clock();
    }

    //
    // Acquiring now must return the most recently completed frame.
    //
    quint8* ptrReadFrame = vicII.acquireReadyFrame();

    QVERIFY(ptrReadFrame != nullptr);
    QCOMPARE(ptrReadFrame[0], quint8(0x0E));
}
