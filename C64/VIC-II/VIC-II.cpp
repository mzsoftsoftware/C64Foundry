#include "VIC-II.h"

#include "C64/Bus/C64Bus.h"


VICII::VICII()
{
    updateTiming();
}
VICII::~VICII()
{
    for (quint8 index = 0; index < 3; ++index)
        delete[] m_ptrFrameBuffers[index];
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
        updateVerticalBorderTiming();

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
        updateHorizontalBorderTiming();
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
void VICII::setTiming(const C64::Timing& timing)
{
    m_timing = timing;
    updateTiming();
}
void VICII::updateTiming()
{
    m_pixelsPerLine = static_cast<quint16>(m_timing.cyclesPerLine * 8);
    updateHorizontalBorderTiming();

    const quint32 frameBufferSize = static_cast<quint32>(m_timing.cyclesPerLine * 8) * m_timing.linesPerFrame;
    if (frameBufferSize != m_frameBufferSize)
    {
        for (quint8 index = 0; index < 3; ++index)
        {
            delete[] m_ptrFrameBuffers[index];
            m_ptrFrameBuffers[index] = new quint8[frameBufferSize];
        }

        m_frameBufferSize = frameBufferSize;
        m_frameBufferIndex = 0;
        m_writeFrameBufferIndex = 0;
        m_readFrameBufferIndex = 2;
        m_writeFrameGeneration = 0;
        m_readFrameGeneration = 0;
        m_readyFrameState.store(frameBufferState(1, 0), std::memory_order_relaxed);
        m_ptrWriteFrameBuffer = m_ptrFrameBuffers[m_writeFrameBufferIndex];
    }
    m_frameBufferIndex = 0;
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
    // Update the vertical border at cycle 63.
    //
    if (m_rasterCycle == 63)
    {
        if (m_rasterLine == m_borderBottom)
            m_verticalBorder = true;

        if (m_rasterLine == m_borderTop &&
            (m_controlRegister1 & 0x10))
        {
            m_verticalBorder = false;
        }
    }

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
    // Cycles 16 through 55 perform the 40 graphics accesses.
    //
    if ((m_rasterCycle >= 16) &&
        (m_rasterCycle <= 55))
    {
        if (m_displayState)
        {
            const quint8 position = m_rasterCycle - 16;

            m_graphicsData =
                readCharacterMemory(m_videoMatrixLine[position],
                                    m_rowCounter);

            //
            // Latch the color belonging to the current character.
            //
            m_graphicsColor = m_colorLine[position];

            //
            // Each graphics access in display state advances
            // VC and VMLI.
            //
            ++m_videoCounter;
            ++m_videoMatrixLineIndex;
        }
        else
        {
            //
            // In idle state, graphics accesses read from $3FFF.
            //
            if (m_controlRegister1 & 0x40)
                m_graphicsData = readMemory(0x39FF);
            else
                m_graphicsData = readMemory(0x3FFF);

            //
            // Video matrix data is zero in idle state.
            //
            m_graphicsColor = 0;
        }

        //
        // Load the graphics shift register with the graphics data
        // fetched by the graphics access.
        //
        m_graphicsShiftRegister = m_graphicsData;
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

    //
    // One VIC-II clock cycle consists of eight pixel clocks.
    //
    for (quint8 pixel = 0; pixel < 8; ++pixel)
        clockGraphicsPixel();

    //
    // Wrap the frame buffer after the last pixel of the frame.
    //
    if (m_frameBufferIndex >= m_frameBufferSize)
    {
        ++m_writeFrameGeneration;
        const quint32 oldState = m_readyFrameState.exchange(frameBufferState(m_writeFrameBufferIndex, m_writeFrameGeneration), std::memory_order_acq_rel);
        m_writeFrameBufferIndex = frameBufferIndex(oldState);
        m_ptrWriteFrameBuffer = m_ptrFrameBuffers[m_writeFrameBufferIndex];
        m_frameBufferIndex = 0;
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

void VICII::clockGraphicsPixel()
{
    //
    // Update the main border flip-flop.
    //
    if (m_rasterX == m_borderRight)
        m_mainBorder = true;

    if (m_rasterX == m_borderLeft)
    {
        //
        // The vertical border is checked again at the
        // left border comparison.
        //
        if (m_rasterLine == m_borderBottom)
            m_verticalBorder = true;

        if (m_rasterLine == m_borderTop &&
            (m_controlRegister1 & 0x10))
        {
            m_verticalBorder = false;
        }

        //
        // Open the main border only if the vertical
        // border is not active.
        //
        if (!m_verticalBorder)
            m_mainBorder = false;
    }

    //
    // Generate and store the graphics pixel for the current phase.
    //
    if (m_graphicsShiftRegister & 0x80)
    {
        m_graphicsPixels[m_graphicsPixelPhase] = m_graphicsColor;
    }
    else
    {
        m_graphicsPixels[m_graphicsPixelPhase] =
            m_colorRegisters[1] & 0x0F;
    }

    //
    // Select the final output pixel.
    //
    if (m_mainBorder)
    {
        m_outputPixel = m_colorRegisters[0] & 0x0F;
    }
    else
    {
        m_outputPixel = m_graphicsPixels[m_graphicsPixelPhase];
    }

    //
    // Store the final output pixel in the frame buffer.
    //
    m_ptrWriteFrameBuffer[m_frameBufferIndex++] = m_outputPixel;

    //
    // Advance the graphics shift register.
    //
    m_graphicsShiftRegister <<= 1;
    m_graphicsPixelPhase = (m_graphicsPixelPhase + 1) & 0x07;

    ++m_rasterX;
    if (m_rasterX >= m_pixelsPerLine)
        m_rasterX = 0;
}

void VICII::updateHorizontalBorderTiming()
{
    //
    // CSEL selects the horizontal display width.
    // Cache the resulting border positions so the
    // pixel pipeline does not have to evaluate CSEL.
    //
    if (m_controlRegister2 & 0x08)
    {
        m_borderLeft = m_timing.borderLeft40;
        m_borderRight = m_timing.borderRight40;
    }
    else
    {
        m_borderLeft = m_timing.borderLeft38;
        m_borderRight = m_timing.borderRight38;
    }
}
void VICII::updateVerticalBorderTiming()
{
    if (m_controlRegister1 & 0x08)
    {
        m_borderTop = 51;
        m_borderBottom = 251;
    }
    else
    {
        m_borderTop = 55;
        m_borderBottom = 247;
    }
}

quint8 VICII::readyFramePixel(quint16 x, quint16 y) const
{
    const quint32 state = m_readyFrameState.load(std::memory_order_acquire);
    return m_ptrFrameBuffers[frameBufferIndex(state)][static_cast<quint32>(y) * m_pixelsPerLine + x];
}

const quint8* VICII::acquireReadyFrame()
{
    quint32 readyState = m_readyFrameState.load(std::memory_order_acquire);
    while (frameBufferGeneration(readyState) != m_readFrameGeneration)
    {
        const quint32 desiredState = frameBufferState(m_readFrameBufferIndex, frameBufferGeneration(readyState));
        if (m_readyFrameState.compare_exchange_weak(readyState, desiredState, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            m_readFrameBufferIndex = frameBufferIndex(readyState);
            m_readFrameGeneration = frameBufferGeneration(readyState);
            return m_ptrFrameBuffers[m_readFrameBufferIndex];
        }

        //
        // compare_exchange_weak() has updated readyState.
        // Retry with the newest published frame.
        //
    }

    return nullptr;
}

const quint8* VICII::peekReadyFrame() const
{
    const quint32 readyState = m_readyFrameState.load(std::memory_order_acquire);
    return m_ptrFrameBuffers[frameBufferIndex(readyState)];
}
