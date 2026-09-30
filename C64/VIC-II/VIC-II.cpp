#include "VIC-II.h"


VICII::VICII()
{
}
VICII::~VICII()
{
}

quint8 VICII::readRegister(const quint8 address) const
{
    if (address <= 0x0F)
        return m_spritePositionRegisters[address];
    if (address == 0x10)
        return m_spriteXMSB;
    if (address == 0x11)
        return m_controlRegister1 | static_cast<quint8>((m_rasterLine & 0x0100) >> 1);
    if (address == 0x12)
        return static_cast<quint8>(m_rasterLine);
    if (address == 0x15)
        return m_spriteEnable;
    if (address == 0x19)
    {
        const quint8 activeInterrupts = m_interruptStatus & m_interruptMask & 0x0F;
        return m_interruptStatus | (activeInterrupts ? 0x80 : 0x00);
    }
    if (address == 0x1A)
        return m_interruptMask;
    if ((address >= 0x20) && (address <= 0x2E))
        return m_colorRegisters[address - 0x20];
    if ((address >= 0x2F) && (address <= 0x3F))
        return 0xFF;

    return 0x00;
}

void VICII::writeRegister(const quint8 address, const quint8 value)
{
    if (address <= 0x0F)
    {
        m_spritePositionRegisters[address] = value;
        return;
    }
    if (address == 0x10)
    {
        m_spriteXMSB = value;
        return;
    }
    if (address == 0x11)
    {
        m_controlRegister1 = value & 0x7F;
        //
        // Bit 7 contains raster compare bit 8 when written.
        //
        m_rasterCompare = (m_rasterCompare & 0x00FF) | (static_cast<quint16>(value & 0x80) << 1);
        return;
    }
    if (address == 0x12)
    {
        m_rasterCompare = (m_rasterCompare & 0x0100) | value;
        return;
    }
    if (address == 0x15)
    {
        m_spriteEnable = value;
        return;
    }
    if (address == 0x19)
    {
        //
        // Interrupt status bits are cleared by writing a one.
        //
        m_interruptStatus &= static_cast<quint8>(~value);
        return;
    }
    if (address == 0x1A)
    {
        //
        // Bits 0-3 enable the VIC-II interrupt sources.
        // Unused bits 4-7 read back as one.
        //
        m_interruptMask = 0xF0 | (value & 0x0F);
        return;
    }
    if ((address >= 0x20) && (address <= 0x2E))
    {
        m_colorRegisters[address - 0x20] = 0xF0 | (value & 0x0F);
        return;
    }
}


void VICII::clock()
{
    ++m_rasterCycle;

    if (m_rasterCycle >= m_timing.cyclesPerLine)
    {
        m_rasterCycle = 0;
        ++m_rasterLine;
        if (m_rasterLine >= m_timing.linesPerFrame)
            m_rasterLine = 0;
        //
        // Set the raster interrupt status when the current raster
        // line matches the raster compare value.
        //
        if (m_rasterLine == m_rasterCompare)
            m_interruptStatus |= 0x01;
    }
}

