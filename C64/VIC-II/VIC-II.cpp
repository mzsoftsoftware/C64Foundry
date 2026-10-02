#include "VIC-II.h"

#include "C64/Bus/C64Bus.h"


VICII::VICII()
{
}
VICII::~VICII()
{
}

quint8 VICII::readRegister(const quint8 address) const
{
    //
    // Sprite position registers: $00-$0F.
    //
    if (address <= 0x0F)
        return m_spritePositionRegisters[address];

    //
    // Color registers: $20-$2E.
    //
    if ((address >= 0x20) && (address <= 0x2E))
        return m_colorRegisters[address - 0x20];

    switch (address)
    {
    case 0x10:
        return m_spriteXMSB;
    case 0x11:
        return m_controlRegister1 | static_cast<quint8>((m_rasterLine & 0x0100) >> 1);
    case 0x12:
        return static_cast<quint8>(m_rasterLine);
    case 0x15:
        return m_spriteEnable;
    case 0x16:
        return m_controlRegister2;
    case 0x17:
        return m_spriteYExpansion;
    case 0x18:
        return m_memoryPointers;
    case 0x19:
    {
        const quint8 activeInterrupts = m_interruptStatus & m_interruptMask & 0x0F;
        return m_interruptStatus | (activeInterrupts ? 0x80 : 0x00);
    }

    case 0x1A:
        return m_interruptMask;
    case 0x1B:
        return m_spriteDataPriority;
    case 0x1C:
        return m_spriteMulticolor;
    case 0x1D:
        return m_spriteXExpansion;
    default:
        break;
    }

    //
    // $2F-$3F are unused.
    //
    if (address >= 0x2F)
        return 0xFF;

    return 0x00;
}

void VICII::writeRegister(const quint8 address, const quint8 value)
{
    //
    // Sprite position registers: $00-$0F.
    //
    if (address <= 0x0F)
    {
        m_spritePositionRegisters[address] = value;
        return;
    }

    //
    // Color registers: $20-$2E.
    //
    if ((address >= 0x20) && (address <= 0x2E))
    {
        m_colorRegisters[address - 0x20] = 0xF0 | (value & 0x0F);
        return;
    }

    switch (address)
    {
    case 0x10:
        m_spriteXMSB = value;
        return;

    case 0x11:
        m_controlRegister1 = value & 0x7F;

        //
        // Bit 7 contains raster compare bit 8 when written.
        //
        m_rasterCompare =
            (m_rasterCompare & 0x00FF)
            | (static_cast<quint16>(value & 0x80) << 1);

        //
        // YSCROLL affects the badline state of the current
        // raster line.
        //
        m_badLine = badLine();
        return;

    case 0x12:
        m_rasterCompare = (m_rasterCompare & 0x0100) | value;
        return;
    case 0x15:
        m_spriteEnable = value;
        return;
    case 0x16:
        //
        // Bits 0-5 contain the VIC-II control state.
        // Unused bits 6-7 read back as one.
        //
        m_controlRegister2 = 0xC0 | (value & 0x3F);
        return;
    case 0x17:
        m_spriteYExpansion = value;
        return;
    case 0x18:
        //
        // Bits 1-7 contain the VIC-II memory pointer configuration.
        // Unused bit 0 reads back as one.
        //
        m_memoryPointers = 0x01 | (value & 0xFE);
        //
        // Bits 4-7 select the Video Matrix base address
        // within the 16 KiB VIC-II address space.
        //
        m_videoMatrixBaseAddress = static_cast<quint16>(value & 0xF0) << 6;
        //
        // Bits 1-3 select the Character Generator base address
        // within the 16 KiB VIC-II address space.
        //
        m_characterBaseAddress = static_cast<quint16>(value & 0x0E) << 10;
        return;
    case 0x19:
        //
        // Interrupt status bits are cleared by writing a one.
        //
        m_interruptStatus &= static_cast<quint8>(~value);
        return;
    case 0x1A:
        //
        // Bits 0-3 enable the VIC-II interrupt sources.
        // Unused bits 4-7 read back as one.
        //
        m_interruptMask = 0xF0 | (value & 0x0F);
        return;
    case 0x1B:
        m_spriteDataPriority = value;
        return;
    case 0x1C:
        m_spriteMulticolor = value;
        return;
    case 0x1D:
        m_spriteXExpansion = value;
        return;
    default:
        return;
    }
}

bool VICII::badLine() const
{
    if (!m_badLinesEnabled)
        return false;
    if ((m_rasterLine < 0x30) || (m_rasterLine > 0xF7))
        return false;
    return (m_rasterLine & 0x07) == (m_controlRegister1 & 0x07);
}

void VICII::clock()
{
    ++m_rasterCycle;

    //
    // Cycle 14 initializes the video matrix sequencer.
    //
    if (m_rasterCycle == 14)
    {
        m_videoCounter = m_videoCounterBase;
        m_videoMatrixLineIndex = 0;

        //
        // A badline resets the row counter and starts
        // the display state.
        //
        if (m_badLine)
        {
            m_rowCounter = 0;
            m_displayState = true;
        }
    }

    //
    // During display state, cycles 16 through 55 perform
    // the 40 graphics accesses.
    //
    if (m_displayState &&
        (m_rasterCycle >= 16) &&
        (m_rasterCycle <= 55))
    {
        const quint8 position = m_rasterCycle - 16;
        m_graphicsData = readCharacterMemory(m_videoMatrixLine[position], m_rowCounter);

        //
        // Load the graphics shift register with the graphics data
        // fetched for the current character.
        //
        m_graphicsShiftRegister = m_graphicsData;

        //
        // Latch the color belonging to the current character.
        //
        m_graphicsColor = m_colorLine[position];

        //
        // Each graphics access advances VC and VMLI.
        //
        ++m_videoCounter;
        ++m_videoMatrixLineIndex;
    }

    //
    // Cycles 15 through 54 of a badline perform the
    // 40 c-accesses for the current character row.
    //
    if (m_badLine &&
        (m_rasterCycle >= 15) &&
        (m_rasterCycle <= 54))
    {
        //
        // VC selects the video matrix and Color RAM address.
        // VMLI selects the position in the internal line buffers.
        //
        m_videoMatrixLine[m_videoMatrixLineIndex] = readVideoMatrixMemory(m_videoCounter);
        m_colorLine[m_videoMatrixLineIndex] = readColorMemory(m_videoCounter);
    }

    //
    // At cycle 58, RC is incremented while it is below 7.
    // When RC has reached 7, VCBASE is updated and the
    // VIC-II leaves the display state.
    //
    if (m_rasterCycle == 58)
    {
        if (m_rowCounter == 7)
        {
            m_videoCounterBase = m_videoCounter;
            m_displayState = false;
        }
        else
        {
            ++m_rowCounter;
        }
    }

    //
    // Advance to the next raster line.
    //
    if (m_rasterCycle >= m_timing.cyclesPerLine)
    {
        m_rasterCycle = 0;
        ++m_rasterLine;

        if (m_rasterLine >= m_timing.linesPerFrame)
            m_rasterLine = 0;

        //
        // DEN on raster line $30 enables badlines for the
        // current display frame.
        //
        if (m_rasterLine == 0x30)
            m_badLinesEnabled = (m_controlRegister1 & 0x10) != 0;

        //
        // Cache the badline state for the new raster line.
        //
        m_badLine = badLine();

        //
        // Set the raster interrupt status when the current raster
        // line matches the raster compare value.
        //
        if (m_rasterLine == m_rasterCompare)
            m_interruptStatus |= 0x01;
    }
}

quint8 VICII::readMemory(const quint16 address)
{
    return m_ptrBus->readVIC(address);
}
quint8 VICII::readVideoMatrixMemory(const quint16 position)
{
    return readMemory(m_videoMatrixBaseAddress + position);
}
quint8 VICII::readCharacterMemory(const quint8 characterCode, const quint8 row)
{
    const quint16 address = m_characterBaseAddress + (static_cast<quint16>(characterCode) << 3) + row;
    return readMemory(address);
}
quint8 VICII::readColorMemory(const quint16 position)
{
    return m_ptrBus->readVICColor(position);
}

quint8 VICII::graphicsPixel() const
{
    //
    // In standard text mode, a set graphics bit selects
    // the character color. A clear bit selects the
    // background color from $D021.
    //
    if (m_graphicsShiftRegister & 0x80)
        return m_graphicsColor;

    return m_colorRegisters[1] & 0x0F;
}
void VICII::clockGraphicsPixel()
{
    m_graphicsShiftRegister <<= 1;
}
