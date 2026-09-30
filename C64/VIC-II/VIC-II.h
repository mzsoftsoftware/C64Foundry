#pragma once

#include <QtGlobal>

#include "C64/C64Timing.h"


class VICII
{
public:
    explicit VICII();
    virtual ~VICII();

    // Getter
    bool irq() const                                        { return (m_interruptStatus & m_interruptMask & 0x0F) != 0; }
    quint8 readRegister(quint8 address) const;

    // Setter
    void setTiming(const C64::Timing& timing)               { m_timing = timing; }
    void writeRegister(quint8 address, quint8 value);

    // Operations
    void clock();

private:
    quint8 m_spritePositionRegisters[0x10] = {};
    quint8 m_spriteXMSB = 0x00;

    quint8 m_controlRegister1 = 0x00;

    quint8 m_colorRegisters[0x0F] = {
        0xF0, 0xF0, 0xF0, 0xF0,
        0xF0, 0xF0, 0xF0, 0xF0,
        0xF0, 0xF0, 0xF0, 0xF0,
        0xF0, 0xF0, 0xF0
    };


    C64::Timing m_timing = C64::PALTiming;
    quint16 m_rasterLine = 0;
    quint8 m_rasterCycle = 0;
    quint16 m_rasterCompare = 0x0000;
    quint8 m_interruptStatus = 0x00;
    quint8 m_interruptMask = 0xF0;
};
