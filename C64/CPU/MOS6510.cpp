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

void MOS6510::reset()
{
    m_state = CpuState::Fetch;

    // PC aus dem Reset-Vector holen
    m_programCounter = static_cast<quint16>(m_ptrBus->read(0xFFFC)) | (static_cast<quint16>(m_ptrBus->read(0xFFFD)) << 8);
    m_accumulator = 0x00;
    m_x = 0x00;
    m_y = 0x00;
    m_stackPointer = 0xFF;
    m_status = 0x20;
    
    m_opcode = 0x00;
    m_address = 0x0000;
    m_data = 0x00;

    m_operation = MOS6510Operation::Unknown;
    m_addressingMode = MOS6510AddressingMode::Implied;
    m_microOperationCount = 0;
    m_microOperationIndex = 0;

    m_pageCrossed = false;
    m_dummyReadPending = false;
}

void MOS6510::clock()
{
    if (m_ptrBus == nullptr)
        return;

    switch (m_state)
    {
    case CpuState::Fetch:
        fetchOpcode();
        decodeInstruction();

        if (m_operation == MOS6510Operation::Unknown)
        {
            qDebug() << "MOS6510: unknown opcode" << Qt::hex << m_opcode << "at" << m_programCounter - 1;
            m_state = CpuState::Stopped;
            break;
        }

        prepareMicroOperations();
        m_state = CpuState::Execute;
        break;

    case CpuState::Execute:
        if (m_dummyReadPending)
        {
            m_dummyReadPending = false;
            break;
        }
        executeMicroOperation(m_microOperations[m_microOperationIndex]);
        ++m_microOperationIndex;
        if(m_microOperationIndex >= m_microOperationCount)
        {
            m_state = CpuState::Fetch;
        }
        break;

    case CpuState::Stopped:
        break;
    }
}

void MOS6510::fetchOpcode()
{
    m_opcode = m_ptrBus->read(m_programCounter);
    ++m_programCounter;
}

void MOS6510::decodeInstruction()
{
    const MOS6510Instruction& instruction = m_instructionTable.instruction(m_opcode);
    m_operation = instruction.operation;
    m_addressingMode = instruction.addressingMode;
}

void MOS6510::prepareMicroOperations()
{
    const MOS6510Instruction& instruction = m_instructionTable.instruction(m_opcode);
    m_microOperationCount = instruction.microOperationCount;
    m_microOperationIndex = 0;
    for (quint8 i = 0; i < m_microOperationCount; ++i)
    {
        m_microOperations[i] = instruction.microOperations[i];
    }
    m_pageCrossed = false;
    m_dummyReadPending = false;
    m_pageCrossingCycle = instruction.pageCrossingCycle;
}

void MOS6510::executeMicroOperation(MOS6510MicroOperation microOperation)
{
    switch (microOperation)
    {
    case MOS6510MicroOperation::NoOperation:
    {
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
    case MOS6510MicroOperation::ReadZeroPageIndexedAddress:
    {
        const quint8 baseAddress = static_cast<quint8>(m_address);
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
        ++m_x;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::IncrementYRegister:
    {
        ++m_y;
        updateLoadFlags(m_y);
        break;
    }

    case MOS6510MicroOperation::DecrementXRegister:
    {
        --m_x;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::DecrementYRegister:
    {
        --m_y;
        updateLoadFlags(m_y);
        break;
    }
    case MOS6510MicroOperation::TransferAccumulatorToXRegister:
    {
        m_x = m_accumulator;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::TransferAccumulatorToYRegister:
    {
        m_y = m_accumulator;
        updateLoadFlags(m_y);
        break;
    }

    case MOS6510MicroOperation::TransferXRegisterToAccumulator:
    {
        m_accumulator = m_x;
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::TransferYRegisterToAccumulator:
    {
        m_accumulator = m_y;
        updateLoadFlags(m_accumulator);
        break;
    }

    case MOS6510MicroOperation::TransferStackPointerToXRegister:
    {
        m_x = m_stackPointer;
        updateLoadFlags(m_x);
        break;
    }

    case MOS6510MicroOperation::TransferXRegisterToStackPointer:
    {
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
    case MOS6510MicroOperation::SetDecimalFlag:
    {
        setStatusFlag(MOS6510StatusFlag::Decimal, true);
        break;
    }
    case MOS6510MicroOperation::ClearDecimalFlag:
    {
        setStatusFlag(MOS6510StatusFlag::Decimal, false);
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
