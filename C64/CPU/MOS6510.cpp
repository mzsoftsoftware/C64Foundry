#include "MOS6510.h"

#include <QDebug>

#include "C64/Bus/C64Bus.h"


MOS6510::MOS6510()
{
}
MOS6510::~MOS6510()
{
}

void MOS6510::setBus(C64Bus* ptrBus)
{
    m_ptrBus = ptrBus;
}

void MOS6510::initialize()
{
    m_state = CpuState::Fetch;
    m_resetCycle = 0;
    m_irqLine = false;
    m_irqPending = false;
    m_irqPolled = false;
    m_initialFetch = true;
    m_irqCycle = 0;
    m_nmiLine = false;
    m_nmiPending = false;
    m_nmiAccepted = false;
    m_nmiCycle = 0;
    m_nmiDelay = false;
    m_nmiVectorFetch = false;
    m_nmiHijack = false;

    m_accumulator = 0x00;
    m_x = 0x00;
    m_y = 0x00;
    m_stackPointer = 0xFF;
    m_programCounter = 0x0000;
    m_status = 0x20;

    m_opcode = 0x00;
    m_address = 0x0000;
    m_data = 0x00;

    m_operation = MOS6510Operation::Unknown;
    m_addressingMode = MOS6510AddressingMode::Implied;
    m_ptrInstruction = nullptr;
    m_microOperationCount = 0;
    m_microOperationIndex = 0;

    m_pageCrossed = false;
    m_dummyReadPending = false;
    m_pageCrossingCycle = false;
}

void MOS6510::reset()
{
    m_resetCycle = 0;
    m_irqPending = false;
    m_irqPolled = false;
    m_initialFetch = false;
    m_irqCycle = 0;
    m_nmiPending = false;
    m_nmiAccepted = false;
    m_nmiCycle = 0;
    m_nmiDelay = false;
    m_nmiVectorFetch = false;
    m_nmiHijack = false;
    m_state = CpuState::Reset;
}

void MOS6510::clock()
{
    if (m_ptrBus == nullptr)
        return;

    switch (m_state)
    {
    case CpuState::Fetch:
        if ((m_nmiAccepted ||
             (m_initialFetch && m_nmiPending)) &&
            !m_nmiDelay)
        {
            m_initialFetch = false;
            m_nmiPending = false;
            m_nmiAccepted = false;

            //
            // NMI has priority over an IRQ that was accepted
            // at the same instruction boundary.
            //
            m_irqPending = false;

            m_nmiCycle = 0;
            m_state = CpuState::Nmi;

            executeNmiCycle();
            break;
        }

        if (m_irqPending ||
            (m_initialFetch &&
             m_irqLine &&
             !statusFlag(MOS6510StatusFlag::InterruptDisable)))
        {
            m_irqPending = false;
            m_irqCycle = 0;
            m_state = CpuState::Irq;

            executeIrqCycle();
            break;
        }

        m_initialFetch = false;

        m_opcode = m_ptrBus->read(m_programCounter);
        ++m_programCounter;

        m_ptrInstruction = &m_instructionTable.instruction(m_opcode);

        m_operation = m_ptrInstruction->operation;
        m_addressingMode = m_ptrInstruction->addressingMode;
        m_microOperationCount = m_ptrInstruction->microOperationCount;

        if (m_operation == MOS6510Operation::Unknown)
        {
            qDebug() << "MOS6510: unknown opcode"
                     << Qt::hex
                     << m_opcode
                     << "at"
                     << static_cast<quint16>(m_programCounter - 1);

            m_state = CpuState::Stopped;
            break;
        }

        m_microOperationIndex = 0;
        m_pageCrossed = false;
        m_dummyReadPending = false;
        m_pageCrossingCycle = m_ptrInstruction->pageCrossingCycle;

        //
        // The opcode-fetch cycle is the interrupt-poll cycle
        // for two-cycle instructions.
        //
        pollIrq();
        if (m_microOperationCount == 1)
            pollNmi();

        m_state = CpuState::Execute;
        break;
    case CpuState::Execute:
    {
        if (m_dummyReadPending)
        {
            const quint16 dummyAddress = static_cast<quint16>(m_address - 0x0100);
            m_ptrBus->read(dummyAddress);
            m_dummyReadPending = false;
            break;
        }

        const bool nmiPollMicroOperation = m_microOperationCount > 1 && m_microOperationIndex + 2 == m_microOperationCount;
        const bool finalMicroOperation = m_microOperationIndex + 1 >= m_microOperationCount;
        if (finalMicroOperation)
        {
            switch (m_operation)
            {
            case MOS6510Operation::CLI:
            case MOS6510Operation::PLP:
            case MOS6510Operation::BCC:
            case MOS6510Operation::BCS:
            case MOS6510Operation::BEQ:
            case MOS6510Operation::BMI:
            case MOS6510Operation::BNE:
            case MOS6510Operation::BPL:
            case MOS6510Operation::BVC:
            case MOS6510Operation::BVS:
                //
                // I changes after the interrupt poll.
                // Keep the poll result from the previous cycle.
                //
                break;
            default:
                pollIrq();
                break;
            }
        }
        if (nmiPollMicroOperation)
        {
            switch (m_operation)
            {
            case MOS6510Operation::BCC:
            case MOS6510Operation::BCS:
            case MOS6510Operation::BEQ:
            case MOS6510Operation::BMI:
            case MOS6510Operation::BNE:
            case MOS6510Operation::BPL:
            case MOS6510Operation::BVC:
            case MOS6510Operation::BVS:
            case MOS6510Operation::BRK:
                //
                // Branches have their own interrupt polling.
                //
                break;

            default:
                pollNmi();
                break;
            }
        }

        executeMicroOperation(m_ptrInstruction->microOperations[m_microOperationIndex]);
        ++m_microOperationIndex;

        if (m_microOperationIndex >= m_microOperationCount)
        {
            if (m_nmiDelay &&
                m_operation != MOS6510Operation::BRK)
            {
                m_nmiDelay = false;
            }

            m_irqPending = m_irqPolled;
            m_state = CpuState::Fetch;
        }
        break;
    }
    case CpuState::Reset:
        executeResetCycle();
        break;

    case CpuState::Irq:
        executeIrqCycle();
        break;

    case CpuState::Nmi:
        executeNmiCycle();
        break;

    case CpuState::Stopped:
        break;
    }
}

void MOS6510::executeResetCycle()
{
    switch (m_resetCycle)
    {
    case 0:
        // C1: Dummy read from current PC
        m_ptrBus->read(m_programCounter);
        break;

    case 1:
        // C2: Dummy read from current PC
        m_ptrBus->read(m_programCounter);
        break;

    case 2:
        // C3: Stack read, no write
        m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        --m_stackPointer;
        break;

    case 3:
        // C4: Stack read, no write
        m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        --m_stackPointer;
        break;

    case 4:
        // C5: Stack read, no write
        m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        --m_stackPointer;
        setStatusFlag(MOS6510StatusFlag::InterruptDisable, true);
        break;

    case 5:
        // C6: Read reset vector low
        m_programCounter = static_cast<quint16>(m_ptrBus->read(0xFFFC));
        break;

    case 6:
        // C7: Read reset vector high
        m_programCounter |= static_cast<quint16>(m_ptrBus->read(0xFFFD)) << 8;
        m_state = CpuState::Fetch;
        return;
    }

    ++m_resetCycle;
}

void MOS6510::executeIrqCycle()
{
    switch (m_irqCycle)
    {
    case 0:
        //
        // C1
        // Suppressed opcode fetch.
        //
        m_ptrBus->read(m_programCounter);
        break;

    case 1:
        //
        // C2
        // Second dummy read from the current PC.
        //
        m_ptrBus->read(m_programCounter);
        break;

    case 2:
        //
        // C3
        // Push program counter high byte.
        //
        m_ptrBus->write(
            static_cast<quint16>(0x0100 | m_stackPointer),
            static_cast<quint8>(m_programCounter >> 8));

        --m_stackPointer;
        break;

    case 3:
        //
        // C4
        // Push program counter low byte.
        //
        m_ptrBus->write(
            static_cast<quint16>(0x0100 | m_stackPointer),
            static_cast<quint8>(m_programCounter & 0x00FF));

        --m_stackPointer;
        break;

    case 4:
        //
        // C5
        // Push status with B clear and U set.
        //
        m_ptrBus->write(
            static_cast<quint16>(0x0100 | m_stackPointer),
            static_cast<quint8>((m_status & 0xEF) | 0x20));

        --m_stackPointer;
        break;

    case 5:
        //
        // C6
        // Set I and select the interrupt vector.
        //
        setStatusFlag(
            MOS6510StatusFlag::InterruptDisable,
            true);

        m_nmiHijack = m_nmiPending;

        if (m_nmiHijack)
        {
            m_nmiPending = false;

            m_programCounter =
                static_cast<quint16>(m_ptrBus->read(0xFFFA));
        }
        else
        {
            m_programCounter =
                static_cast<quint16>(m_ptrBus->read(0xFFFE));
        }

        //
        // From now until vector high, a newly arriving
        // NMI is too late to hijack this interrupt.
        //
        m_nmiVectorFetch = true;
        break;

    case 6:
        //
        // C7
        // Read vector high byte.
        //
        if (m_nmiHijack)
        {
            m_programCounter |=
                static_cast<quint16>(
                    m_ptrBus->read(0xFFFB)) << 8;
        }
        else
        {
            m_programCounter |=
                static_cast<quint16>(
                    m_ptrBus->read(0xFFFF)) << 8;
        }

        m_nmiVectorFetch = false;
        m_nmiHijack = false;
        m_state = CpuState::Fetch;
        return;
    }

    ++m_irqCycle;
}

void MOS6510::pollIrq()
{
    m_irqPolled = m_irqLine && !statusFlag(MOS6510StatusFlag::InterruptDisable);
}


void MOS6510::setNmiLine(const bool active)
{
    if (active && !m_nmiLine)
    {
        //
        // Rising logical edge = assertion of /NMI.
        //
        m_nmiPending = true;

        if (m_nmiVectorFetch)
            m_nmiDelay = true;
    }
    else if (!active && m_nmiLine)
    {
        //
        // An NMI pulse that starts and ends inside the
        // protected vector-fetch window is lost.
        //
        if (m_nmiDelay && m_nmiVectorFetch)
        {
            m_nmiPending = false;
            m_nmiDelay = false;
        }
    }

    m_nmiLine = active;
}
void MOS6510::executeNmiCycle()
{
    switch (m_nmiCycle)
    {
    case 0:
        //
        // C1
        // Suppressed opcode fetch.
        //
        m_ptrBus->read(m_programCounter);
        break;

    case 1:
        //
        // C2
        // Second dummy read from the current PC.
        //
        m_ptrBus->read(m_programCounter);
        break;

    case 2:
        //
        // C3
        // Push program counter high byte.
        //
        m_ptrBus->write(
            static_cast<quint16>(0x0100 | m_stackPointer),
            static_cast<quint8>(m_programCounter >> 8));

        --m_stackPointer;
        break;

    case 3:
        //
        // C4
        // Push program counter low byte.
        //
        m_ptrBus->write(
            static_cast<quint16>(0x0100 | m_stackPointer),
            static_cast<quint8>(m_programCounter & 0x00FF));

        --m_stackPointer;
        break;

    case 4:
        //
        // C5
        // Push status with B clear and U set.
        //
        m_ptrBus->write(
            static_cast<quint16>(0x0100 | m_stackPointer),
            static_cast<quint8>((m_status & 0xEF) | 0x20));

        --m_stackPointer;
        break;

    case 5:
        //
        // C6
        // Set I and read NMI vector low byte.
        //
        setStatusFlag(
            MOS6510StatusFlag::InterruptDisable,
            true);

        m_programCounter =
            static_cast<quint16>(m_ptrBus->read(0xFFFA));
        break;

    case 6:
        //
        // C7
        // Read NMI vector high byte.
        //
        m_programCounter |=
            static_cast<quint16>(
                m_ptrBus->read(0xFFFB)) << 8;

        m_state = CpuState::Fetch;
        return;
    }

    ++m_nmiCycle;
}
void MOS6510::pollNmi()
{
    if (m_nmiPending)
    {
        m_nmiPending = false;
        m_nmiAccepted = true;
    }
}

void MOS6510::executeMicroOperation(MOS6510MicroOperation microOperation)
{
    switch (microOperation)
    {
    case MOS6510MicroOperation::NoOperation:
    {
        m_ptrBus->read(m_programCounter);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateToAccumulator:
    {
        const quint16 address = m_programCounter;
        m_accumulator = m_ptrBus->read(address);
        ++m_programCounter;
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateToXRegister:
    {
        const quint16 address = m_programCounter;
        m_x = m_ptrBus->read(address);
        ++m_programCounter;
        updateLoadFlags(m_x);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateToYRegister:
    {
        const quint16 address = m_programCounter;
        m_y = m_ptrBus->read(address);
        ++m_programCounter;
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateAndAccumulator:
    {
        const quint16 address = m_programCounter;
        m_accumulator &= m_ptrBus->read(address);
        ++m_programCounter;
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateOrAccumulator:
    {
        const quint16 address = m_programCounter;
        m_accumulator |= m_ptrBus->read(address);
        ++m_programCounter;
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateExclusiveOrAccumulator:
    {
        const quint16 address = m_programCounter;
        m_accumulator ^= m_ptrBus->read(address);
        ++m_programCounter;
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateAddToAccumulator:
    {
        const quint16 address = m_programCounter;
        const quint8 operand = m_ptrBus->read(address);
        ++m_programCounter;
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateSubtractFromAccumulator:
    {
        const quint16 address = m_programCounter;
        const quint8 operand = m_ptrBus->read(address);
        ++m_programCounter;
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateCompareAccumulator:
    {
        const quint16 address = m_programCounter;
        const quint8 operand = m_ptrBus->read(address);
        ++m_programCounter;
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateCompareXRegister:
    {
        const quint16 address = m_programCounter;
        const quint8 operand = m_ptrBus->read(address);
        ++m_programCounter;
        const quint8 x = m_x;
        const quint8 result = static_cast<quint8>(x - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, x >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadImmediateCompareYRegister:
    {
        const quint16 address = m_programCounter;
        const quint8 operand = m_ptrBus->read(address);
        ++m_programCounter;
        const quint8 y = m_y;
        const quint8 result = static_cast<quint8>(y - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, y >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageAddress:
    {
        m_address = m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageToAccumulator:
    {
        m_accumulator = m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageToXRegister:
    {
        m_x = m_ptrBus->read(m_address);
        updateLoadFlags(m_x);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageToYRegister:
    {
        m_y = m_ptrBus->read(m_address);
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageAndAccumulator:
    {
        m_accumulator &= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageOrAccumulator:
    {
        m_accumulator |= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageExclusiveOrAccumulator:
    {
        m_accumulator ^= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageAddToAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageSubtractFromAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageCompareAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageCompareXRegister:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 x = m_x;
        const quint8 result = static_cast<quint8>(x - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, x >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageCompareYRegister:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 y = m_y;
        const quint8 result = static_cast<quint8>(y - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, y >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageBitTest:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        setStatusFlag(MOS6510StatusFlag::Zero, (m_accumulator & operand) == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (operand & 0x80) != 0);
        setStatusFlag(MOS6510StatusFlag::Overflow, (operand & 0x40) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedAddress:
    {
        const quint8 baseAddress = static_cast<quint8>(m_address);
        // NMOS 6502/6510 performs a dummy read from the
        // unindexed zero-page address during this cycle.
        m_ptrBus->read(baseAddress);
        switch (m_addressingMode)
        {
        case MOS6510AddressingMode::ZeroPageX:
        case MOS6510AddressingMode::IndexedIndirect:
            m_address = static_cast<quint8>(baseAddress + m_x);
            break;
        case MOS6510AddressingMode::ZeroPageY:
            m_address = static_cast<quint8>(baseAddress + m_y);
            break;
        default:
            break;
        }
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedToAccumulator:
    {
        m_accumulator = m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedToXRegister:
    {
        m_x = m_ptrBus->read(m_address);
        updateLoadFlags(m_x);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedToYRegister:
    {
        m_y = m_ptrBus->read(m_address);
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedAndAccumulator:
    {
        m_accumulator &= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ReadZeroPageIndexedOrAccumulator:
    {
        m_accumulator |= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ReadZeroPageIndexedExclusiveOrAccumulator:
    {
        m_accumulator ^= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedAddToAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedSubtractFromAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadZeroPageIndexedCompareAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteAddressLow:
    {
        m_address = m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteAddressHigh:
    {
        const quint8 highByte = m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        m_address |= static_cast<quint16>(highByte) << 8;
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteXAddress:
    {
        const quint8 highByte = m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        m_address |= static_cast<quint16>(highByte) << 8;
        const quint16 baseAddress = m_address;
        m_address += m_x;
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed && m_pageCrossingCycle)
        {
            m_dummyReadPending = true;
        }
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteYAddress:
    {
        const quint8 highByte = m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        m_address |= static_cast<quint16>(highByte) << 8;
        const quint16 baseAddress = m_address;
        m_address += m_y;
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed && m_pageCrossingCycle)
        {
            m_dummyReadPending = true;
        }
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteBitTest:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        setStatusFlag(MOS6510StatusFlag::Zero, (m_accumulator & operand) == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (operand & 0x80) != 0);
        setStatusFlag(MOS6510StatusFlag::Overflow, (operand & 0x40) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedDummy:
    {
        quint16 dummyAddress = m_address;
        if (m_pageCrossed)
        {
            dummyAddress = static_cast<quint16>(dummyAddress - 0x0100);
        }
        m_ptrBus->read(dummyAddress);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteToAccumulator:
    {
        m_accumulator = m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteToXRegister:
    {
        m_x = m_ptrBus->read(m_address);
        updateLoadFlags(m_x);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteToYRegister:
    {
        m_y = m_ptrBus->read(m_address);
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteAndAccumulator:
    {
        m_accumulator &= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteOrAccumulator:
    {
        m_accumulator |= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteExclusiveOrAccumulator:
    {
        m_accumulator ^= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteAddToAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteSubtractFromAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteCompareAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteCompareXRegister:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 x = m_x;
        const quint8 result = static_cast<quint8>(x - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, x >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteCompareYRegister:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 y = m_y;
        const quint8 result = static_cast<quint8>(y - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, y >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedToAccumulator:
    {
        m_accumulator = m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedToXRegister:
    {
        m_x = m_ptrBus->read(m_address);
        updateLoadFlags(m_x);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedToYRegister:
    {
        m_y = m_ptrBus->read(m_address);
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedAndAccumulator:
    {
        m_accumulator &= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedOrAccumulator:
    {
        m_accumulator |= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedExclusiveOrAccumulator:
    {
        m_accumulator ^= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedAddToAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedSubtractFromAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteIndexedCompareAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectAddressLow:
    {
        m_data = m_ptrBus->read(m_address);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectAddressHigh:
    {
        const quint8 highByte = m_ptrBus->read(static_cast<quint8>(m_address + 1));
        m_address = static_cast<quint16>(m_data) | (static_cast<quint16>(highByte) << 8);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectAddressHighIndexed:
    {
        const quint8 highByte = m_ptrBus->read(static_cast<quint8>(m_address + 1));
        m_address = static_cast<quint16>(m_data) | (static_cast<quint16>(highByte) << 8);
        const quint16 baseAddress = m_address;
        m_address += m_y;
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed && m_pageCrossingCycle)
        {
            m_dummyReadPending = true;
        }
        break;
    }
    case MOS6510MicroOperation::ReadIndirectToAccumulator:
    {
        const quint8 value = m_ptrBus->read(m_address);
        m_accumulator = value;
        updateLoadFlags(value);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectAddToAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectSubtractFromAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectCompareAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectAndAccumulator:
    {
        m_accumulator &= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ReadIndirectOrAccumulator:
    {
        m_accumulator |= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ReadIndirectExclusiveOrAccumulator:
    {
        m_accumulator ^= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectIndexedToAccumulator:
    {
        const quint8 value = m_ptrBus->read(m_address);
        m_accumulator = value;
        updateLoadFlags(value);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectIndexedAddToAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        addToAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectIndexedSubtractFromAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        subtractFromAccumulator(operand);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectIndexedCompareAccumulator:
    {
        const quint8 operand = m_ptrBus->read(m_address);
        const quint8 accumulator = m_accumulator;
        const quint8 result = static_cast<quint8>(accumulator - operand);
        setStatusFlag(MOS6510StatusFlag::Carry, accumulator >= operand);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectIndexedAndAccumulator:
    {
        m_accumulator &= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ReadIndirectIndexedOrAccumulator:
    {
        m_accumulator |= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ReadIndirectIndexedExclusiveOrAccumulator:
    {
        m_accumulator ^= m_ptrBus->read(m_address);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::WriteAccumulator:
    {
        m_ptrBus->write(m_address, m_accumulator);
        break;
    }
    case MOS6510MicroOperation::WriteXRegister:
    {
        m_ptrBus->write(m_address, m_x);
        break;
    }
    case MOS6510MicroOperation::WriteYRegister:
    {
        m_ptrBus->write(m_address, m_y);
        break;
    }
    case MOS6510MicroOperation::IncrementXRegister:
    {
        m_ptrBus->read(m_programCounter);
        ++m_x;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::IncrementYRegister:
    {
        m_ptrBus->read(m_programCounter);
        ++m_y;
        updateLoadFlags(m_y);
        break;
    }

    case MOS6510MicroOperation::DecrementXRegister:
    {
        m_ptrBus->read(m_programCounter);
        --m_x;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::DecrementYRegister:
    {
        m_ptrBus->read(m_programCounter);
        --m_y;
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::ReadMemoryToData:
    {
        m_data = m_ptrBus->read(m_address);
        break;
    }
    case MOS6510MicroOperation::WriteDataToMemory:
    {
        m_ptrBus->write(m_address, m_data);
        break;
    }
    case MOS6510MicroOperation::IncrementDataAndWriteToMemory:
    {
        ++m_data;
        m_ptrBus->write(m_address, m_data);
        updateLoadFlags(m_data);
        break;
    }
    case MOS6510MicroOperation::DecrementDataAndWriteToMemory:
    {
        --m_data;
        m_ptrBus->write(m_address, m_data);
        updateLoadFlags(m_data);
        break;
    }
    case MOS6510MicroOperation::ShiftLeftAccumulator:
    {
        m_ptrBus->read(m_programCounter);
        const bool carry = (m_accumulator & 0x80) != 0;
        m_accumulator = static_cast<quint8>(m_accumulator << 1);
        setStatusFlag(MOS6510StatusFlag::Carry, carry);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ShiftRightAccumulator:
    {
        m_ptrBus->read(m_programCounter);
        const bool carry = (m_accumulator & 0x01) != 0;
        m_accumulator = static_cast<quint8>(m_accumulator >> 1);
        setStatusFlag(MOS6510StatusFlag::Carry, carry);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::RotateLeftAccumulator:
    {
        m_ptrBus->read(m_programCounter);
        const bool carryIn = (m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0;
        const bool carryOut = (m_accumulator & 0x80) != 0;
        m_accumulator = static_cast<quint8>((m_accumulator << 1) | (carryIn ? 0x01 : 0x00));
        setStatusFlag(MOS6510StatusFlag::Carry, carryOut);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::RotateRightAccumulator:
    {
        m_ptrBus->read(m_programCounter);
        const bool carryIn = (m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0;
        const bool carryOut = (m_accumulator & 0x01) != 0;
        m_accumulator = static_cast<quint8>((m_accumulator >> 1) | (carryIn ? 0x80 : 0x00));
        setStatusFlag(MOS6510StatusFlag::Carry, carryOut);
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::ShiftLeftDataAndWriteToMemory:
    {
        const bool carry = (m_data & 0x80) != 0;
        m_data = static_cast<quint8>(m_data << 1);
        m_ptrBus->write(m_address, m_data);
        setStatusFlag(MOS6510StatusFlag::Carry, carry);
        updateLoadFlags(m_data);
        break;
    }

    case MOS6510MicroOperation::ShiftRightDataAndWriteToMemory:
    {
        const bool carry = (m_data & 0x01) != 0;
        m_data = static_cast<quint8>(m_data >> 1);
        m_ptrBus->write(m_address, m_data);
        setStatusFlag(MOS6510StatusFlag::Carry, carry);
        updateLoadFlags(m_data);
        break;
    }

    case MOS6510MicroOperation::RotateLeftDataAndWriteToMemory:
    {
        const bool carryIn = (m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0;
        const bool carryOut = (m_data & 0x80) != 0;
        m_data = static_cast<quint8>((m_data << 1) | (carryIn ? 0x01 : 0x00));
        m_ptrBus->write(m_address, m_data);
        setStatusFlag(MOS6510StatusFlag::Carry, carryOut);
        updateLoadFlags(m_data);
        break;
    }

    case MOS6510MicroOperation::RotateRightDataAndWriteToMemory:
    {
        const bool carryIn = (m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0;
        const bool carryOut = (m_data & 0x01) != 0;
        m_data = static_cast<quint8>((m_data >> 1) | (carryIn ? 0x80 : 0x00));
        m_ptrBus->write(m_address, m_data);
        setStatusFlag(MOS6510StatusFlag::Carry, carryOut);
        updateLoadFlags(m_data);
        break;
    }
    case MOS6510MicroOperation::TransferAccumulatorToXRegister:
    {
        m_ptrBus->read(m_programCounter);
        m_x = m_accumulator;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::TransferAccumulatorToYRegister:
    {
        m_ptrBus->read(m_programCounter);
        m_y = m_accumulator;
        updateLoadFlags(m_y);
        break;
    }

    case MOS6510MicroOperation::TransferXRegisterToAccumulator:
    {
        m_ptrBus->read(m_programCounter);
        m_accumulator = m_x;
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::TransferYRegisterToAccumulator:
    {
        m_ptrBus->read(m_programCounter);
        m_accumulator = m_y;
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::TransferStackPointerToXRegister:
    {
        m_ptrBus->read(m_programCounter);
        m_x = m_stackPointer;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::TransferXRegisterToStackPointer:
    {
        m_ptrBus->read(m_programCounter);
        m_stackPointer = m_x;
        break;
    }
    case MOS6510MicroOperation::WriteAccumulatorToStack:
    {
        m_ptrBus->write(0x0100 | m_stackPointer, m_accumulator);
        --m_stackPointer;
        break;
    }
    case MOS6510MicroOperation::ReadStackToAccumulator:
    {
        ++m_stackPointer;
        m_accumulator = m_ptrBus->read(0x0100 | m_stackPointer);
        updateLoadFlags(m_accumulator);
        break;
    }
    case MOS6510MicroOperation::WriteStatusToStack:
    {
        m_ptrBus->write(0x0100 | m_stackPointer, m_status | 0x30);
        --m_stackPointer;
        break;
    }
    case MOS6510MicroOperation::ReadStackToStatus:
    {
        ++m_stackPointer;
        const quint8 value = m_ptrBus->read(0x0100 | m_stackPointer);
        m_status = value | 0x20;
        break;
    }
    case MOS6510MicroOperation::ReadStackDummy:
    {
        m_ptrBus->read(0x0100 | m_stackPointer);
        break;
    }
    case MOS6510MicroOperation::SetDecimalFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::Decimal, true);
        break;
    }
    case MOS6510MicroOperation::ClearDecimalFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::Decimal, false);
        break;
    }
    case MOS6510MicroOperation::ClearCarryFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::Carry, false);
        break;
    }
    case MOS6510MicroOperation::SetCarryFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::Carry, true);
        break;
    }
    case MOS6510MicroOperation::ClearInterruptDisableFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::InterruptDisable, false);
        break;
    }
    case MOS6510MicroOperation::SetInterruptDisableFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::InterruptDisable, true);
        break;
    }
    case MOS6510MicroOperation::ClearOverflowFlag:
    {
        m_ptrBus->read(m_programCounter);
        setStatusFlag(MOS6510StatusFlag::Overflow, false);
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchCarryClear:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BCC: branch only if Carry is clear.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchCarrySet:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BCS: branch only if Carry is set.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) == 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchEqual:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BEQ: branch only if Zero is set.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Zero)) == 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchNotEqual:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BNE: branch only if Zero is clear.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Zero)) != 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }

        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchMinus:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BMI: branch only if Negative is set.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Negative)) == 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchPlus:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BPL: branch only if Negative is clear.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Negative)) != 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchOverflowClear:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BVC: branch only if Overflow is clear.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Overflow)) != 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::ReadRelativeBranchOverflowSet:
    {
        pollIrq();
        pollNmi();
        const qint8 offset = static_cast<qint8>(m_ptrBus->read(m_programCounter));
        ++m_programCounter;
        // BVS: branch only if Overflow is set.
        if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Overflow)) == 0)
        {
            m_microOperationIndex = m_microOperationCount - 1;
            break;
        }
        const quint16 baseAddress = m_programCounter;
        m_address = static_cast<quint16>(static_cast<qint32>(baseAddress) + static_cast<qint32>(offset));
        m_pageCrossed = (baseAddress & 0xFF00) != (m_address & 0xFF00);
        if (m_pageCrossed)
        {
            m_data = static_cast<quint8>(baseAddress >> 8);
        }
        break;
    }
    case MOS6510MicroOperation::Branch:
    {
        // Taken branch always performs a dummy read from the
        // instruction following the branch operand.
        m_ptrBus->read(m_programCounter);
        m_programCounter = m_address;
        if (!m_pageCrossed)
        {
            m_microOperationIndex = m_microOperationCount - 1;
        }
        break;
    }
    case MOS6510MicroOperation::BranchPageCrossing:
    {
        pollIrq();
        pollNmi();
        const quint16 dummyAddress = static_cast<quint16>((static_cast<quint16>(m_data) << 8) | (m_address & 0x00FF));
        m_ptrBus->read(dummyAddress);
        break;
    }
    case MOS6510MicroOperation::ReadAbsoluteAddressHighAndJump:
    {
        const quint8 highByte = m_ptrBus->read(m_programCounter);
        m_address |= static_cast<quint16>(highByte) << 8;
        m_programCounter = m_address;

        break;
    }
    case MOS6510MicroOperation::ReadIndirectJumpAddressLow:
    {
        m_data = m_ptrBus->read(m_address);
        break;
    }
    case MOS6510MicroOperation::ReadIndirectJumpAddressHigh:
    {
        const quint16 highAddress = static_cast<quint16>((m_address & 0xFF00) | static_cast<quint8>(m_address + 1));
        const quint8 highByte = m_ptrBus->read(highAddress);
        m_programCounter = static_cast<quint16>(static_cast<quint16>(highByte) << 8 | m_data);
        break;
    }
    case MOS6510MicroOperation::ReadJsrAddressLow:
    {
        m_address = m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        break;
    }
    case MOS6510MicroOperation::ReadJsrStackDummy:
    {
        m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        break;
    }
    case MOS6510MicroOperation::WriteJsrReturnAddressHigh:
    {
        m_ptrBus->write(static_cast<quint16>(0x0100 | m_stackPointer), static_cast<quint8>(m_programCounter >> 8));
        --m_stackPointer;
        break;
    }
    case MOS6510MicroOperation::WriteJsrReturnAddressLow:
    {
        m_ptrBus->write(static_cast<quint16>(0x0100 | m_stackPointer), static_cast<quint8>(m_programCounter & 0x00FF));
        --m_stackPointer;
        break;
    }
    case MOS6510MicroOperation::ReadJsrAddressHighAndJump:
    {
        const quint8 highByte = m_ptrBus->read(m_programCounter);
        m_address |= static_cast<quint16>(highByte) << 8;
        m_programCounter = m_address;
        break;
    }
    case MOS6510MicroOperation::ReadRtsProgramCounterDummy:
    {
        m_ptrBus->read(m_programCounter);
        break;
    }
    case MOS6510MicroOperation::ReadRtsStackDummy:
    {
        m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        break;
    }
    case MOS6510MicroOperation::ReadRtsReturnAddressLow:
    {
        ++m_stackPointer;
        m_address = m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        break;
    }
    case MOS6510MicroOperation::ReadRtsReturnAddressHigh:
    {
        ++m_stackPointer;
        const quint8 highByte = m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        m_address |= static_cast<quint16>(highByte) << 8;
        m_programCounter = m_address;
        break;
    }
    case MOS6510MicroOperation::RtsIncrementProgramCounter:
    {
        m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        break;
    }
    case MOS6510MicroOperation::ReadRtiProgramCounterDummy:
    {
        m_ptrBus->read(m_programCounter);
        break;
    }
    case MOS6510MicroOperation::ReadRtiStackDummy:
    {
        m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        break;
    }
    case MOS6510MicroOperation::ReadRtiStatus:
    {
        ++m_stackPointer;
        const quint8 value = m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        m_status = value | 0x20;
        break;
    }
    case MOS6510MicroOperation::ReadRtiProgramCounterLow:
    {
        ++m_stackPointer;
        m_address = m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        break;
    }
    case MOS6510MicroOperation::ReadRtiProgramCounterHigh:
    {
        ++m_stackPointer;
        const quint8 highByte = m_ptrBus->read(static_cast<quint16>(0x0100 | m_stackPointer));
        m_programCounter = static_cast<quint16>((static_cast<quint16>(highByte) << 8) | m_address);
        break;
    }
    case MOS6510MicroOperation::ReadBrkPadding:
    {
        m_ptrBus->read(m_programCounter);
        ++m_programCounter;
        break;
    }
    case MOS6510MicroOperation::WriteBrkProgramCounterHigh:
    {
        m_ptrBus->write(static_cast<quint16>(0x0100 | m_stackPointer), static_cast<quint8>(m_programCounter >> 8));
        --m_stackPointer;
        break;
    }
    case MOS6510MicroOperation::WriteBrkProgramCounterLow:
    {
        m_ptrBus->write(static_cast<quint16>(0x0100 | m_stackPointer), static_cast<quint8>(m_programCounter & 0x00FF));
        --m_stackPointer;
        break;
    }
    case MOS6510MicroOperation::WriteBrkStatus:
    {
        m_ptrBus->write(static_cast<quint16>(0x0100 | m_stackPointer), static_cast<quint8>(m_status | 0x30));
        --m_stackPointer;
        setStatusFlag(MOS6510StatusFlag::InterruptDisable, true);
        break;
    }
    case MOS6510MicroOperation::ReadBrkVectorLow:
    {
        m_nmiHijack = m_nmiPending;

        if (m_nmiHijack)
        {
            m_nmiPending = false;
            m_address = m_ptrBus->read(0xFFFA);
        }
        else
        {
            m_address = m_ptrBus->read(0xFFFE);
        }
        m_nmiVectorFetch = true;
        break;
    }
    case MOS6510MicroOperation::ReadBrkVectorHigh:
    {
        quint8 highByte;

        if (m_nmiHijack)
            highByte = m_ptrBus->read(0xFFFB);
        else
            highByte = m_ptrBus->read(0xFFFF);

        m_programCounter =
            static_cast<quint16>(
                (static_cast<quint16>(highByte) << 8) |
                m_address);

        m_nmiVectorFetch = false;
        m_nmiHijack = false;
        break;
    }
    }
}

void MOS6510::setStatusFlag(const MOS6510StatusFlag flag, const bool value)
{
    const quint8 mask = static_cast<quint8>(flag);
    if (value)
        m_status |= mask;
    else
        m_status &= static_cast<quint8>(~mask);
}
bool MOS6510::statusFlag(const MOS6510StatusFlag flag) const
{
    const quint8 mask = static_cast<quint8>(flag);
    return (m_status & mask) != 0;
}

void MOS6510::updateLoadFlags(const quint8 value)
{
    setStatusFlag(MOS6510StatusFlag::Zero, value == 0);
    setStatusFlag(MOS6510StatusFlag::Negative, (value & 0x80) != 0);
}

void MOS6510::addToAccumulator(quint8 operand)
{
    const quint8 accumulator = m_accumulator;
    const quint8 carryIn = (m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0 ? 1 : 0;

    //
    // Binary Mode
    //
    if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Decimal)) == 0)
    {
        const quint16 sum = static_cast<quint16>(accumulator) + static_cast<quint16>(operand) + static_cast<quint16>(carryIn);
        const quint8 result = static_cast<quint8>(sum);
        const bool carry = sum > 0xFF;
        const bool overflow = ((~(accumulator ^ operand)) & (accumulator ^ result) & 0x80) != 0;
        m_accumulator = result;
        setStatusFlag(MOS6510StatusFlag::Carry, carry);
        setStatusFlag(MOS6510StatusFlag::Zero, result == 0);
        setStatusFlag(MOS6510StatusFlag::Negative, (result & 0x80) != 0);
        setStatusFlag(MOS6510StatusFlag::Overflow, overflow);
        return;
    }

    //
    // NMOS 6502/6510 Decimal Mode
    //
    quint16 decimal = static_cast<quint16>(accumulator & 0x0F) + static_cast<quint16>(operand & 0x0F) + static_cast<quint16>(carryIn);
    if (decimal > 9)
        decimal += 6;
    decimal = (decimal & 0x0F) + static_cast<quint16>(accumulator & 0xF0) + static_cast<quint16>(operand & 0xF0) + ((decimal > 0x0F) ? 0x10 : 0x00);

    //
    // Z is determined from the unadjusted binary result.
    //
    const quint16 binarySum = static_cast<quint16>(accumulator) + static_cast<quint16>(operand) + static_cast<quint16>(carryIn);
    const bool zero = static_cast<quint8>(binarySum) == 0;

    //
    // N and V are determined before the final
    // high-digit decimal correction.
    //
    const bool negative = (decimal & 0x80) != 0;
    const bool overflow = (((accumulator ^ decimal) & 0x80) != 0) && (((accumulator ^ operand) & 0x80) == 0);
    if ((decimal & 0x1F0) > 0x90)
        decimal += 0x60;
    const bool carry = (decimal & 0xFF0) > 0xF0;
    m_accumulator = static_cast<quint8>(decimal);
    setStatusFlag(MOS6510StatusFlag::Carry, carry);
    setStatusFlag(MOS6510StatusFlag::Zero, zero);
    setStatusFlag(MOS6510StatusFlag::Negative, negative);
    setStatusFlag(MOS6510StatusFlag::Overflow, overflow);
}
void MOS6510::subtractFromAccumulator(quint8 operand)
{
    const quint8 accumulator = m_accumulator;
    const quint8 carryIn = (m_status & static_cast<quint8>(MOS6510StatusFlag::Carry)) != 0 ? 1 : 0;

    //
    // The NMOS 6502/6510 determines N, V, Z and C for SBC
    // from the binary subtraction, even in Decimal Mode.
    //
    const quint16 difference = static_cast<quint16>(accumulator) - static_cast<quint16>(operand) - static_cast<quint16>(1 - carryIn);
    const quint8 binaryResult = static_cast<quint8>(difference);
    const bool carry = difference < 0x100;
    const bool overflow = ((accumulator ^ operand) & (accumulator ^ binaryResult) & 0x80) != 0;
    setStatusFlag(MOS6510StatusFlag::Carry, carry);
    setStatusFlag(MOS6510StatusFlag::Zero, binaryResult == 0);
    setStatusFlag(MOS6510StatusFlag::Negative, (binaryResult & 0x80) != 0);
    setStatusFlag(MOS6510StatusFlag::Overflow, overflow);

    //
    // Binary Mode
    //
    if ((m_status & static_cast<quint8>(MOS6510StatusFlag::Decimal)) == 0)
    {
        m_accumulator = binaryResult;
        return;
    }

    //
    // NMOS 6502/6510 Decimal Mode
    //
    unsigned decimal = static_cast<unsigned>(accumulator & 0x0F) - static_cast<unsigned>(operand & 0x0F) - (carryIn ? 0U : 1U);
    if (decimal & 0x10)
    {
        decimal = ((decimal - 6U) & 0x0F) | (static_cast<unsigned>(accumulator & 0xF0) - static_cast<unsigned>(operand & 0xF0) - 0x10U);
    }
    else
    {
        decimal = (decimal & 0x0F) | (static_cast<unsigned>(accumulator & 0xF0) - static_cast<unsigned>(operand & 0xF0));
    }
    if (decimal & 0x100)
        decimal -= 0x60;
    m_accumulator = static_cast<quint8>(decimal);
}
