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
    initializeStoreInstructions();
    initializeTransferInstructions();
    //initializeArithmeticInstructions();
    //initializeLogicInstructions();
    //initializeCompareInstructions();
    //initializeIncrementInstructions();
    //initializeShiftInstructions();
    //initializeBranchInstructions();
    //initializeStackInstructions();
    //initializeJumpInstructions();
    //initializeFlagInstructions();
    initializeSpecialInstructions();
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

    m_instructions[0xBD].operation = MOS6510Operation::LDA;
    m_instructions[0xBD].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xBD].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xBD].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHighIndexed;
    m_instructions[0xBD].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToAccumulator;
    m_instructions[0xBD].microOperationCount = 3;

    m_instructions[0xBE].operation = MOS6510Operation::LDX;
    m_instructions[0xBE].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0xBE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xBE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHighIndexed;
    m_instructions[0xBE].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToXRegister;
    m_instructions[0xBE].microOperationCount = 3;

    m_instructions[0xBC].operation = MOS6510Operation::LDY;
    m_instructions[0xBC].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xBC].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xBC].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHighIndexed;
    m_instructions[0xBC].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToYRegister;
    m_instructions[0xBC].microOperationCount = 3;

    m_instructions[0xB9].operation = MOS6510Operation::LDA;
    m_instructions[0xB9].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0xB9].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xB9].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHighIndexed;
    m_instructions[0xB9].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToAccumulator;
    m_instructions[0xB9].microOperationCount = 3;

    m_instructions[0xA1].operation = MOS6510Operation::LDA;
    m_instructions[0xA1].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0xA1].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xA1].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xA1].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0xA1].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0xA1].microOperations[4] = MOS6510MicroOperation::ReadIndirectToAccumulator;
    m_instructions[0xA1].microOperationCount = 5;

    m_instructions[0xB1].operation = MOS6510Operation::LDA;
    m_instructions[0xB1].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0xB1].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xB1].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0xB1].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighIndexed;
    m_instructions[0xB1].microOperations[3] = MOS6510MicroOperation::ReadIndirectIndexedToAccumulator;
    m_instructions[0xB1].microOperationCount = 4;
}

void MOS6510InstructionTable::initializeStoreInstructions()
{
    m_instructions[0x85].operation = MOS6510Operation::STA;
    m_instructions[0x85].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x85].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x85].microOperations[1] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x85].microOperationCount = 2;

    m_instructions[0x95].operation = MOS6510Operation::STA;
    m_instructions[0x95].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x95].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x95].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x95].microOperations[2] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x95].microOperationCount = 3;

    m_instructions[0x8D].operation = MOS6510Operation::STA;
    m_instructions[0x8D].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x8D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x8D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x8D].microOperations[2] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x8D].microOperationCount = 3;

    m_instructions[0x9D].operation = MOS6510Operation::STA;
    m_instructions[0x9D].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x9D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x9D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHighIndexed;
    m_instructions[0x9D].microOperations[2] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x9D].microOperationCount = 3;

    m_instructions[0x99].operation = MOS6510Operation::STA;
    m_instructions[0x99].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0x99].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x99].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHighIndexed;
    m_instructions[0x99].microOperations[2] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x99].microOperationCount = 3;

    m_instructions[0x81].operation = MOS6510Operation::STA;
    m_instructions[0x81].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0x81].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x81].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x81].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x81].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0x81].microOperations[4] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x81].microOperationCount = 5;

    m_instructions[0x91].operation = MOS6510Operation::STA;
    m_instructions[0x91].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0x91].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x91].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x91].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighIndexed;
    m_instructions[0x91].microOperations[3] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x91].microOperationCount = 4;

    m_instructions[0x86].operation = MOS6510Operation::STX;
    m_instructions[0x86].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x86].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x86].microOperations[1] = MOS6510MicroOperation::WriteXRegister;
    m_instructions[0x86].microOperationCount = 2;

    m_instructions[0x96].operation = MOS6510Operation::STX;
    m_instructions[0x96].addressingMode = MOS6510AddressingMode::ZeroPageY;
    m_instructions[0x96].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x96].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x96].microOperations[2] = MOS6510MicroOperation::WriteXRegister;
    m_instructions[0x96].microOperationCount = 3;

    m_instructions[0x8E].operation = MOS6510Operation::STX;
    m_instructions[0x8E].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x8E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x8E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x8E].microOperations[2] = MOS6510MicroOperation::WriteXRegister;
    m_instructions[0x8E].microOperationCount = 3;

    m_instructions[0x84].operation = MOS6510Operation::STY;
    m_instructions[0x84].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x84].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x84].microOperations[1] = MOS6510MicroOperation::WriteYRegister;
    m_instructions[0x84].microOperationCount = 2;

    m_instructions[0x94].operation = MOS6510Operation::STY;
    m_instructions[0x94].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x94].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x94].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x94].microOperations[2] = MOS6510MicroOperation::WriteYRegister;
    m_instructions[0x94].microOperationCount = 3;

    m_instructions[0x8C].operation = MOS6510Operation::STY;
    m_instructions[0x8C].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x8C].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x8C].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x8C].microOperations[2] = MOS6510MicroOperation::WriteYRegister;
    m_instructions[0x8C].microOperationCount = 3;
}

void MOS6510InstructionTable::initializeTransferInstructions()
{
    m_instructions[0xAA].operation = MOS6510Operation::TAX;
    m_instructions[0xAA].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xAA].microOperations[0] = MOS6510MicroOperation::TransferAccumulatorToXRegister;
    m_instructions[0xAA].microOperationCount = 1;

    m_instructions[0xA8].operation = MOS6510Operation::TAY;
    m_instructions[0xA8].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xA8].microOperations[0] = MOS6510MicroOperation::TransferAccumulatorToYRegister;
    m_instructions[0xA8].microOperationCount = 1;

    m_instructions[0x8A].operation = MOS6510Operation::TXA;
    m_instructions[0x8A].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x8A].microOperations[0] = MOS6510MicroOperation::TransferXRegisterToAccumulator;
    m_instructions[0x8A].microOperationCount = 1;

    m_instructions[0x98].operation = MOS6510Operation::TYA;
    m_instructions[0x98].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x98].microOperations[0] = MOS6510MicroOperation::TransferYRegisterToAccumulator;
    m_instructions[0x98].microOperationCount = 1;

    m_instructions[0xBA].operation = MOS6510Operation::TSX;
    m_instructions[0xBA].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xBA].microOperations[0] = MOS6510MicroOperation::TransferStackPointerToXRegister;
    m_instructions[0xBA].microOperationCount = 1;

    m_instructions[0x9A].operation = MOS6510Operation::TXS;
    m_instructions[0x9A].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x9A].microOperations[0] = MOS6510MicroOperation::TransferXRegisterToStackPointer;
    m_instructions[0x9A].microOperationCount = 1;

    m_instructions[0x9A].operation = MOS6510Operation::TXS;
    m_instructions[0x9A].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x9A].microOperations[0] = MOS6510MicroOperation::TransferXRegisterToStackPointer;
    m_instructions[0x9A].microOperationCount = 1;
}

void MOS6510InstructionTable::initializeSpecialInstructions()
{
    m_instructions[0xEA].operation = MOS6510Operation::NOP;
    m_instructions[0xEA].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xEA].microOperations[0] = MOS6510MicroOperation::NoOperation;
    m_instructions[0xEA].microOperationCount = 1;
}