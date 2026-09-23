#include "MOS6510InstructionTable.h"


MOS6510InstructionTable::MOS6510InstructionTable()
{
    for (quint16 opcode = 0; opcode < 256; ++opcode)
    {
        m_instructions[opcode].operation = MOS6510Operation::Unknown;
        m_instructions[opcode].addressingMode = MOS6510AddressingMode::Implied;
        m_instructions[opcode].microOperationCount = 0;
    }

    initializeInstructions();
}
MOS6510InstructionTable::~MOS6510InstructionTable()
{
}


void MOS6510InstructionTable::initializeInstructions()
{
    initializeLoadInstructions();
    //initializeStoreInstructions();
    //initializeTransferInstructions();
    //initializeArithmeticInstructions();
    //initializeLogicInstructions();
    //initializeCompareInstructions();
    //initializeIncrementInstructions();
    //initializeShiftInstructions();
    //initializeBranchInstructions();
    //initializeStackInstructions();
    //initializeJumpInstructions();
    //initializeFlagInstructions();
    //initializeSpecialInstructions();
}


void MOS6510InstructionTable::initializeLoadInstructions()
{
    m_instructions[0xA9].operation = MOS6510Operation::LDA;
    m_instructions[0xA9].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xA9].microOperations[0] = MOS6510MicroOperation::ReadImmediateToAccumulator;
    m_instructions[0xA9].microOperationCount = 1;

    m_instructions[0xA2].operation = MOS6510Operation::LDX;
    m_instructions[0xA2].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xA2].microOperations[0] = MOS6510MicroOperation::ReadImmediateToXRegister;
    m_instructions[0xA2].microOperationCount = 1;

    m_instructions[0xA0].operation = MOS6510Operation::LDY;
    m_instructions[0xA0].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xA0].microOperations[0] = MOS6510MicroOperation::ReadImmediateToYRegister;
    m_instructions[0xA0].microOperationCount = 1;

    m_instructions[0xA5].operation = MOS6510Operation::LDA;
    m_instructions[0xA5].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xA5].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xA5].microOperations[1] = MOS6510MicroOperation::ReadZeroPageToAccumulator;
    m_instructions[0xA5].microOperationCount = 2;

    m_instructions[0xA6].operation = MOS6510Operation::LDX;
    m_instructions[0xA6].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xA6].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xA6].microOperations[1] = MOS6510MicroOperation::ReadZeroPageToXRegister;
    m_instructions[0xA6].microOperationCount = 2;

    m_instructions[0xA4].operation = MOS6510Operation::LDY;
    m_instructions[0xA4].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xA4].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xA4].microOperations[1] = MOS6510MicroOperation::ReadZeroPageToYRegister;
    m_instructions[0xA4].microOperationCount = 2;

    m_instructions[0xB5].operation = MOS6510Operation::LDA;
    m_instructions[0xB5].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0xB5].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xB5].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xB5].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedToAccumulator;
    m_instructions[0xB5].microOperationCount = 3;

    m_instructions[0xB6].operation = MOS6510Operation::LDX;
    m_instructions[0xB6].addressingMode = MOS6510AddressingMode::ZeroPageY;
    m_instructions[0xB6].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xB6].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xB6].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedToXRegister;
    m_instructions[0xB6].microOperationCount = 3;

    m_instructions[0xB4].operation = MOS6510Operation::LDY;
    m_instructions[0xB4].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0xB4].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xB4].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xB4].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedToYRegister;
    m_instructions[0xB4].microOperationCount = 3;

    m_instructions[0xAD].operation = MOS6510Operation::LDA;
    m_instructions[0xAD].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xAD].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xAD].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xAD].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteToAccumulator;
    m_instructions[0xAD].microOperationCount = 3;

    m_instructions[0xAE].operation = MOS6510Operation::LDX;
    m_instructions[0xAE].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xAE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xAE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xAE].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteToXRegister;
    m_instructions[0xAE].microOperationCount = 3;

    m_instructions[0xAC].operation = MOS6510Operation::LDY;
    m_instructions[0xAC].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xAC].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xAC].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xAC].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteToYRegister;
    m_instructions[0xAC].microOperationCount = 3;

}

