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
};
