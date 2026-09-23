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
}

void MOS6510::executeMicroOperation(MOS6510MicroOperation microOperation)
{
    switch (microOperation)
    {
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
    case MOS6510MicroOperation::ReadZeroPageIndexedAddress:
    {
        const quint8 baseAddress = static_cast<quint8>(m_address);
        switch (m_operation)
        {
        case MOS6510Operation::LDA:
        case MOS6510Operation::LDY:
            m_address = static_cast<quint8>(baseAddress + m_x);
            break;
        case MOS6510Operation::LDX:
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