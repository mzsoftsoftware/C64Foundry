#pragma once

#include <QObject>


class VICIITest : public QObject
{
    Q_OBJECT

private slots:
    void testInitialRegisters();

    void testBorderColorRegister();
    void testBackgroundColorRegister();
    void testColorRegisterBoundaries();

    void testSprite0PositionRegisters();
    void testSpriteXMSBRegister();
    void testSpriteEnableRegister();
    void testSpriteYExpansionRegister();
    void testSpriteDataPriorityRegister();
    void testSpriteMulticolorRegister();
    void testSpriteXExpansionRegister();

    void testControlRegister1();
    void testControlRegister2();
    void testRasterCounterRegister();

    void testClockWithinRasterLine();
    void testRasterLineAdvance();
    void testRasterFrameWrap();
    void testRasterCounterBit8();
    void testRasterCompareRegister();
    void testRasterIRQStatus();
    void testRasterCompareBit8();
    void testRasterIRQAcknowledge();

    void testInterruptMaskRegister();
    void testIRQStatusBit();
    void testMaskedRasterIRQ();
    void testEnablePendingRasterIRQ();
    void testIRQLine();
    void testDisablePendingRasterIRQ();

    void testMemoryPointerRegister();

    void testMemoryRead();
    void testVideoMatrixBaseAddress();
    void testCharacterBaseAddress();

    void testCharacterMemoryRead();
    void testVideoMatrixMemoryRead();

    void testBadLineRasterAndYScroll();
    void testBadLineRasterRange();
    void testBadLineEnable();
    void testBadLineEnableWithDEN();

    void testBadLineBA();
    void testNonBadLineBA();
    void testBadLineAEC();
    void testNonBadLineAEC();

    void testBadLineFirstCAccess();
    void testRowCounterIncrement();

    void testFirstGraphicsAccess();
    void testBadLineStartsDisplayState();
    void testDisplayStateEndsAtRowCounterSeven();

    void testFirstGraphicsMemoryAccess();
    void testGraphicsMemoryAccessSequence();
    void testVideoCounterBaseUpdate();
    void testVideoCounterReloadFromBase();

    void testBadLineColorRAMAccess();
    void testCAccessUsesVideoCounter();
    void testCAccessVideoCounterSequence();

    void testFirstGraphicsColor();
    void testStandardTextGraphicsPixel();
    void testStandardTextGraphicsBackgroundPixel();
    void testGraphicsPixelPhase();

    void testRasterXAdvance();
    void testRasterXLineWrap();
    void testClockAdvancesGraphicsPixels();
    void testStandardTextGraphicsPixelBuffer();

    void testRasterXTracksRasterCycle();
    void testHorizontalBorderTiming();
    void testHorizontalBorderRightComparison();

    void testVerticalBorderTiming();
    void testVerticalBorderInitialState();
    void testVerticalBorderComparisons();
    void testVerticalBorderRequiresDEN();

    void testMainBorderOpensAtLeft();
    void testMainBorderStaysClosedAtLeftDuringVerticalBorder();

    void testVerticalBorderOpensAtLeftComparison();
    void testVerticalBorderClosesAtLeftComparison();
    void testVerticalBorderClosesAtCycle63();
    void testRasterLineAdvancesWithRasterXWrap();

    void testBorderPixelUsesBorderColor();
    void testGraphicsPixelIgnoresBorderColor();
    void testOutputPixelUsesGraphicsPixelOutsideBorder();
    void testGraphicsPipelineContinuesDuringBorder();

    void testIdleStateGraphicsAccess();
    void testIdleStateECMGraphicsAccess();
};
