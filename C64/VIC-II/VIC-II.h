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
    quint16 characterBaseAddress() const                    { return m_characterBaseAddress; }
    quint8 rasterCycle() const                              { return m_rasterCycle; }
    bool badLine() const;
    bool ba() const;
    bool aec() const;

    // Setter
    void setBus(C64Bus* ptrBus)                             { m_ptrBus = ptrBus; }
    void setTiming(const C64::Timing& timing)               { m_timing = timing; }
    void writeRegister(quint8 address, quint8 value);

    // Operations
    void clock();

    quint8 readMemory(quint16 address);
    quint8 readVideoMatrixMemory(quint16 position);
    quint8 readCharacterMemory(quint8 characterCode, quint8 row);


private:
    quint8 m_spritePositionRegisters[0x10] = {};
    quint8 m_spriteXMSB = 0x00;
    quint8 m_controlRegister1 = 0x00;
    quint8 m_spriteEnable = 0x00;
    quint8 m_controlRegister2 = 0xC0;
    quint8 m_spriteYExpansion = 0x00;
    quint8 m_memoryPointers = 0x01;
    quint16 m_videoMatrixBaseAddress = 0x0000;
    quint16 m_characterBaseAddress = 0x0000;
    quint8 m_spriteDataPriority = 0x00;
    quint8 m_spriteMulticolor = 0x00;
    quint8 m_spriteXExpansion = 0x00;

    bool m_badLinesEnabled = false;


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
    quint8 m_interruptStatus = 0x00;
    quint8 m_interruptMask = 0xF0;
};
