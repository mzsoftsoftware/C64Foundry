#pragma once

#include <QtGlobal>

#include "C64/C64Timing.h"

class C64Bus;

class VICII
{
public:
    explicit VICII();
    virtual ~VICII();

    // Getter
    bool irq() const                                        { return (m_interruptStatus & m_interruptMask & 0x0F) != 0; }
    quint8 readRegister(quint8 address) const;
    quint16 videoMatrixBaseAddress() const                  { return m_videoMatrixBaseAddress; }
    quint8 videoMatrixLine(quint8 position) const           { return m_videoMatrixLine[position]; }
    quint16 characterBaseAddress() const                    { return m_characterBaseAddress; }
    quint8 rasterLine() const                               { return m_rasterLine; }
    quint8 rasterCycle() const                              { return m_rasterCycle; }
    quint8 rowCounter() const                               { return m_rowCounter; }
    quint16 videoCounter() const                            { return m_videoCounter; }
    quint8 videoMatrixLineIndex() const                     { return m_videoMatrixLineIndex; }
    bool displayState() const                               { return m_displayState; }
    quint8 graphicsData() const                             { return m_graphicsData; }
    quint8 graphicsColor() const                            { return m_graphicsColor; }
    quint8 graphicsPixelPhase() const                       { return m_graphicsPixelPhase; }
    quint16 videoCounterBase() const                        { return m_videoCounterBase; }
    quint8 colorLine(quint8 position) const                 { return m_colorLine[position]; }

    bool badLine() const;
    bool ba() const
    {
        //
        // BA remains high when the current raster line
        // is not a badline.
        //
        if (!m_badLine)
            return true;

        //
        // On a badline, BA goes low three cycles before the first
        // c-access and remains low until all 40 c-accesses are done.
        //
        return (m_rasterCycle < 12) || (m_rasterCycle > 54);
    }
    bool aec() const
    {
        //
        // During a badline, the VIC-II takes over the CPU bus
        // for the 40 c-accesses in cycles 15 through 54.
        //
        if (m_badLine && (m_rasterCycle >= 15) && (m_rasterCycle <= 54))
            return false;

        return true;
    }

    quint8 graphicsPixel() const;


    // Setter
    void setBus(C64Bus* ptrBus)                             { m_ptrBus = ptrBus; }
    void setTiming(const C64::Timing& timing)               { m_timing = timing; }
    void writeRegister(quint8 address, quint8 value);

    // Operations
    void clock();
    void clockGraphicsPixel();

    quint8 readMemory(quint16 address);
    quint8 readVideoMatrixMemory(quint16 position);
    quint8 readCharacterMemory(quint8 characterCode, quint8 row);
    quint8 readColorMemory(const quint16 position);

private:
    quint8 m_spritePositionRegisters[0x10] = {};
    quint8 m_spriteXMSB = 0x00;
    quint8 m_controlRegister1 = 0x00;
    quint8 m_spriteEnable = 0x00;
    quint8 m_controlRegister2 = 0xC0;
    quint8 m_spriteYExpansion = 0x00;
    quint8 m_memoryPointers = 0x01;

    quint16 m_videoCounter = 0;
    quint16 m_videoCounterBase = 0;
    quint16 m_videoMatrixBaseAddress = 0x0000;
    quint8 m_videoMatrixLine[40] = {};
    quint8 m_videoMatrixLineIndex = 0;
    bool m_displayState = false;

    quint8 m_graphicsData = 0;
    quint8 m_graphicsShiftRegister = 0;
    quint8 m_graphicsColor = 0;
    quint8 m_graphicsPixelPhase = 0;
    quint8 m_colorLine[40] = {};

    quint16 m_characterBaseAddress = 0x0000;

    quint8 m_spriteDataPriority = 0x00;
    quint8 m_spriteMulticolor = 0x00;
    quint8 m_spriteXExpansion = 0x00;

    bool m_badLinesEnabled = false;
    bool m_badLine = false;

    quint8 m_colorRegisters[0x0F] = {
        0xF0, 0xF0, 0xF0, 0xF0,
        0xF0, 0xF0, 0xF0, 0xF0,
        0xF0, 0xF0, 0xF0, 0xF0,
        0xF0, 0xF0, 0xF0
    };

    C64Bus* m_ptrBus = nullptr;
    C64::Timing m_timing = C64::PALTiming;

    quint16 m_rasterLine = 0;
    quint8 m_rasterCycle = 0;
    quint16 m_rasterCompare = 0x0000;

    quint8 m_rowCounter = 0;

    quint8 m_interruptStatus = 0x00;
    quint8 m_interruptMask = 0xF0;
};
