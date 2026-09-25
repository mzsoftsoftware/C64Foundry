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
    initializeStackInstructions();
    initializeLogicalInstructions();
    initializeArithmeticInstructions();
    initializeShiftInstructions();
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
    m_instructions[0xBD].pageCrossingCycle = true;
    m_instructions[0xBD].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xBD].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0xBD].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToAccumulator;
    m_instructions[0xBD].microOperationCount = 3;

    m_instructions[0xBE].operation = MOS6510Operation::LDX;
    m_instructions[0xBE].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0xBE].pageCrossingCycle = true;
    m_instructions[0xBE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xBE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0xBE].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToXRegister;
    m_instructions[0xBE].microOperationCount = 3;

    m_instructions[0xBC].operation = MOS6510Operation::LDY;
    m_instructions[0xBC].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xBC].pageCrossingCycle = true;
    m_instructions[0xBC].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xBC].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0xBC].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedToYRegister;
    m_instructions[0xBC].microOperationCount = 3;

    m_instructions[0xB9].operation = MOS6510Operation::LDA;
    m_instructions[0xB9].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0xB9].pageCrossingCycle = true;
    m_instructions[0xB9].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xB9].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
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
    m_instructions[0xB1].pageCrossingCycle = true;
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
    m_instructions[0x9D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x9D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x9D].microOperations[3] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x9D].microOperationCount = 4;

    m_instructions[0x99].operation = MOS6510Operation::STA;
    m_instructions[0x99].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0x99].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x99].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0x99].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x99].microOperations[3] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x99].microOperationCount = 4;

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
    m_instructions[0x91].microOperations[3] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x91].microOperations[4] = MOS6510MicroOperation::WriteAccumulator;
    m_instructions[0x91].microOperationCount = 5;

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
}

void MOS6510InstructionTable::initializeStackInstructions()
{
    m_instructions[0x48].operation = MOS6510Operation::PHA;
    m_instructions[0x48].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x48].microOperations[0] = MOS6510MicroOperation::NoOperation;
    m_instructions[0x48].microOperations[1] = MOS6510MicroOperation::WriteAccumulatorToStack;
    m_instructions[0x48].microOperationCount = 2;

    m_instructions[0x08].operation = MOS6510Operation::PHP;
    m_instructions[0x08].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x08].microOperations[0] = MOS6510MicroOperation::NoOperation;
    m_instructions[0x08].microOperations[1] = MOS6510MicroOperation::WriteStatusToStack;
    m_instructions[0x08].microOperationCount = 2;

    m_instructions[0x68].operation = MOS6510Operation::PLA;
    m_instructions[0x68].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x68].microOperations[0] = MOS6510MicroOperation::NoOperation;
    m_instructions[0x68].microOperations[1] = MOS6510MicroOperation::ReadStackDummy;
    m_instructions[0x68].microOperations[2] = MOS6510MicroOperation::ReadStackToAccumulator;
    m_instructions[0x68].microOperationCount = 3;

    m_instructions[0x28].operation = MOS6510Operation::PLP;
    m_instructions[0x28].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x28].microOperations[0] = MOS6510MicroOperation::NoOperation;
    m_instructions[0x28].microOperations[1] = MOS6510MicroOperation::ReadStackDummy;
    m_instructions[0x28].microOperations[2] = MOS6510MicroOperation::ReadStackToStatus;
    m_instructions[0x28].microOperationCount = 3;
}

void MOS6510InstructionTable::initializeLogicalInstructions()
{
    m_instructions[0x29].operation = MOS6510Operation::AND;
    m_instructions[0x29].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0x29].microOperations[0] = MOS6510MicroOperation::ReadImmediateAndAccumulator;
    m_instructions[0x29].microOperationCount = 1;

    m_instructions[0x09].operation = MOS6510Operation::ORA;
    m_instructions[0x09].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0x09].microOperations[0] = MOS6510MicroOperation::ReadImmediateOrAccumulator;
    m_instructions[0x09].microOperationCount = 1;

    m_instructions[0x49].operation = MOS6510Operation::EOR;
    m_instructions[0x49].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0x49].microOperations[0] = MOS6510MicroOperation::ReadImmediateExclusiveOrAccumulator;
    m_instructions[0x49].microOperationCount = 1;

    m_instructions[0x25].operation = MOS6510Operation::AND;
    m_instructions[0x25].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x25].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x25].microOperations[1] = MOS6510MicroOperation::ReadZeroPageAndAccumulator;
    m_instructions[0x25].microOperationCount = 2;

    m_instructions[0x05].operation = MOS6510Operation::ORA;
    m_instructions[0x05].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x05].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x05].microOperations[1] = MOS6510MicroOperation::ReadZeroPageOrAccumulator;
    m_instructions[0x05].microOperationCount = 2;

    m_instructions[0x45].operation = MOS6510Operation::EOR;
    m_instructions[0x45].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x45].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x45].microOperations[1] = MOS6510MicroOperation::ReadZeroPageExclusiveOrAccumulator;
    m_instructions[0x45].microOperationCount = 2;

    m_instructions[0x35].operation = MOS6510Operation::AND;
    m_instructions[0x35].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x35].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x35].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x35].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedAndAccumulator;
    m_instructions[0x35].microOperationCount = 3;

    m_instructions[0x15].operation = MOS6510Operation::ORA;
    m_instructions[0x15].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x15].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x15].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x15].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedOrAccumulator;
    m_instructions[0x15].microOperationCount = 3;

    m_instructions[0x55].operation = MOS6510Operation::EOR;
    m_instructions[0x55].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x55].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x55].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x55].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedExclusiveOrAccumulator;
    m_instructions[0x55].microOperationCount = 3;

    m_instructions[0x2D].operation = MOS6510Operation::AND;
    m_instructions[0x2D].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x2D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x2D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x2D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteAndAccumulator;
    m_instructions[0x2D].microOperationCount = 3;

    m_instructions[0x0D].operation = MOS6510Operation::ORA;
    m_instructions[0x0D].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x0D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x0D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x0D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteOrAccumulator;
    m_instructions[0x0D].microOperationCount = 3;

    m_instructions[0x4D].operation = MOS6510Operation::EOR;
    m_instructions[0x4D].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x4D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x4D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x4D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteExclusiveOrAccumulator;
    m_instructions[0x4D].microOperationCount = 3;

    m_instructions[0x3D].operation = MOS6510Operation::AND;
    m_instructions[0x3D].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x3D].pageCrossingCycle = true;
    m_instructions[0x3D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x3D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x3D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedAndAccumulator;
    m_instructions[0x3D].microOperationCount = 3;

    m_instructions[0x1D].operation = MOS6510Operation::ORA;
    m_instructions[0x1D].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x1D].pageCrossingCycle = true;
    m_instructions[0x1D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x1D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x1D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedOrAccumulator;
    m_instructions[0x1D].microOperationCount = 3;

    m_instructions[0x5D].operation = MOS6510Operation::EOR;
    m_instructions[0x5D].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x5D].pageCrossingCycle = true;
    m_instructions[0x5D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x5D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x5D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedExclusiveOrAccumulator;
    m_instructions[0x5D].microOperationCount = 3;

    m_instructions[0x39].operation = MOS6510Operation::AND;
    m_instructions[0x39].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0x39].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x39].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0x39].microOperations[2] = MOS6510MicroOperation::AndMemoryWithAccumulator;
    m_instructions[0x39].microOperationCount = 3;
    m_instructions[0x39].pageCrossingCycle = true;

    m_instructions[0x19].operation = MOS6510Operation::ORA;
    m_instructions[0x19].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0x19].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x19].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0x19].microOperations[2] = MOS6510MicroOperation::OrMemoryWithAccumulator;
    m_instructions[0x19].microOperationCount = 3;
    m_instructions[0x19].pageCrossingCycle = true;

    m_instructions[0x59].operation = MOS6510Operation::EOR;
    m_instructions[0x59].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0x59].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x59].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0x59].microOperations[2] = MOS6510MicroOperation::ExclusiveOrMemoryWithAccumulator;
    m_instructions[0x59].microOperationCount = 3;
    m_instructions[0x59].pageCrossingCycle = true;

    m_instructions[0x21].operation = MOS6510Operation::AND;
    m_instructions[0x21].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0x21].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x21].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x21].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x21].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0x21].microOperations[4] = MOS6510MicroOperation::AndMemoryWithAccumulator;
    m_instructions[0x21].microOperationCount = 5;

    m_instructions[0x01].operation = MOS6510Operation::ORA;
    m_instructions[0x01].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0x01].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x01].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x01].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x01].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0x01].microOperations[4] = MOS6510MicroOperation::OrMemoryWithAccumulator;
    m_instructions[0x01].microOperationCount = 5;

    m_instructions[0x41].operation = MOS6510Operation::EOR;
    m_instructions[0x41].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0x41].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x41].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x41].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x41].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0x41].microOperations[4] = MOS6510MicroOperation::ExclusiveOrMemoryWithAccumulator;
    m_instructions[0x41].microOperationCount = 5;

    m_instructions[0x31].operation = MOS6510Operation::AND;
    m_instructions[0x31].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0x31].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x31].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x31].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighAndAddY;
    m_instructions[0x31].microOperations[3] = MOS6510MicroOperation::AndMemoryWithAccumulator;
    m_instructions[0x31].microOperationCount = 4;
    m_instructions[0x31].pageCrossingCycle = true;

    m_instructions[0x11].operation = MOS6510Operation::ORA;
    m_instructions[0x11].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0x11].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x11].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x11].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighAndAddY;
    m_instructions[0x11].microOperations[3] = MOS6510MicroOperation::OrMemoryWithAccumulator;
    m_instructions[0x11].microOperationCount = 4;
    m_instructions[0x11].pageCrossingCycle = true;

    m_instructions[0x51].operation = MOS6510Operation::EOR;
    m_instructions[0x51].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0x51].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x51].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x51].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighAndAddY;
    m_instructions[0x51].microOperations[3] = MOS6510MicroOperation::ExclusiveOrMemoryWithAccumulator;
    m_instructions[0x51].microOperationCount = 4;
    m_instructions[0x51].pageCrossingCycle = true;
}

void MOS6510InstructionTable::initializeArithmeticInstructions()
{
    m_instructions[0x69].operation = MOS6510Operation::ADC;
    m_instructions[0x69].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0x69].microOperations[0] = MOS6510MicroOperation::ReadImmediateAddToAccumulator;
    m_instructions[0x69].microOperationCount = 1;

    m_instructions[0x65].operation = MOS6510Operation::ADC;
    m_instructions[0x65].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x65].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x65].microOperations[1] = MOS6510MicroOperation::ReadZeroPageAddToAccumulator;
    m_instructions[0x65].microOperationCount = 2;

    m_instructions[0x75].operation = MOS6510Operation::ADC;
    m_instructions[0x75].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x75].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x75].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x75].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedAddToAccumulator;
    m_instructions[0x75].microOperationCount = 3;

    m_instructions[0x6D].operation = MOS6510Operation::ADC;
    m_instructions[0x6D].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x6D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x6D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x6D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteAddToAccumulator;
    m_instructions[0x6D].microOperationCount = 3;

    m_instructions[0x7D].operation = MOS6510Operation::ADC;
    m_instructions[0x7D].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x7D].pageCrossingCycle = true;
    m_instructions[0x7D].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x7D].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x7D].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedAddToAccumulator;
    m_instructions[0x7D].microOperationCount = 3;

    m_instructions[0x79].operation = MOS6510Operation::ADC;
    m_instructions[0x79].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0x79].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x79].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0x79].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedAddToAccumulator;
    m_instructions[0x79].microOperationCount = 3;
    m_instructions[0x79].pageCrossingCycle = true;

    m_instructions[0x61].operation = MOS6510Operation::ADC;
    m_instructions[0x61].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0x61].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x61].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x61].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x61].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0x61].microOperations[4] = MOS6510MicroOperation::ReadIndirectAddToAccumulator;
    m_instructions[0x61].microOperationCount = 5;

    m_instructions[0x71].operation = MOS6510Operation::ADC;
    m_instructions[0x71].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0x71].pageCrossingCycle = true;
    m_instructions[0x71].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x71].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0x71].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighIndexed;
    m_instructions[0x71].microOperations[3] = MOS6510MicroOperation::ReadIndirectIndexedAddToAccumulator;
    m_instructions[0x71].microOperationCount = 4;

    m_instructions[0xE9].operation = MOS6510Operation::SBC;
    m_instructions[0xE9].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xE9].microOperations[0] = MOS6510MicroOperation::ReadImmediateSubtractFromAccumulator;
    m_instructions[0xE9].microOperationCount = 1;

    m_instructions[0xE5].operation = MOS6510Operation::SBC;
    m_instructions[0xE5].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xE5].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xE5].microOperations[1] = MOS6510MicroOperation::ReadZeroPageSubtractFromAccumulator;
    m_instructions[0xE5].microOperationCount = 2;

    m_instructions[0xF5].operation = MOS6510Operation::SBC;
    m_instructions[0xF5].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0xF5].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xF5].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xF5].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedSubtractFromAccumulator;
    m_instructions[0xF5].microOperationCount = 3;

    m_instructions[0xED].operation = MOS6510Operation::SBC;
    m_instructions[0xED].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xED].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xED].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xED].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteSubtractFromAccumulator;
    m_instructions[0xED].microOperationCount = 3;

    m_instructions[0xFD].operation = MOS6510Operation::SBC;
    m_instructions[0xFD].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xFD].pageCrossingCycle = true;
    m_instructions[0xFD].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xFD].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0xFD].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedSubtractFromAccumulator;
    m_instructions[0xFD].microOperationCount = 3;

    m_instructions[0xF9].operation = MOS6510Operation::SBC;
    m_instructions[0xF9].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0xF9].pageCrossingCycle = true;
    m_instructions[0xF9].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xF9].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0xF9].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedSubtractFromAccumulator;
    m_instructions[0xF9].microOperationCount = 3;

    m_instructions[0xE1].operation = MOS6510Operation::SBC;
    m_instructions[0xE1].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0xE1].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xE1].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xE1].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0xE1].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0xE1].microOperations[4] = MOS6510MicroOperation::ReadIndirectSubtractFromAccumulator;
    m_instructions[0xE1].microOperationCount = 5;

    m_instructions[0xF1].operation = MOS6510Operation::SBC;
    m_instructions[0xF1].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0xF1].pageCrossingCycle = true;
    m_instructions[0xF1].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xF1].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0xF1].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighIndexed;
    m_instructions[0xF1].microOperations[3] = MOS6510MicroOperation::ReadIndirectIndexedSubtractFromAccumulator;
    m_instructions[0xF1].microOperationCount = 4;

    m_instructions[0xC9].operation = MOS6510Operation::CMP;
    m_instructions[0xC9].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xC9].microOperations[0] = MOS6510MicroOperation::ReadImmediateCompareAccumulator;
    m_instructions[0xC9].microOperationCount = 1;

    m_instructions[0xC5].operation = MOS6510Operation::CMP;
    m_instructions[0xC5].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xC5].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xC5].microOperations[1] = MOS6510MicroOperation::ReadZeroPageCompareAccumulator;
    m_instructions[0xC5].microOperationCount = 2;

    m_instructions[0xD5].operation = MOS6510Operation::CMP;
    m_instructions[0xD5].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0xD5].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xD5].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xD5].microOperations[2] = MOS6510MicroOperation::ReadZeroPageIndexedCompareAccumulator;
    m_instructions[0xD5].microOperationCount = 3;

    m_instructions[0xCD].operation = MOS6510Operation::CMP;
    m_instructions[0xCD].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xCD].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xCD].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xCD].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteCompareAccumulator;
    m_instructions[0xCD].microOperationCount = 3;

    m_instructions[0xDD].operation = MOS6510Operation::CMP;
    m_instructions[0xDD].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xDD].pageCrossingCycle = true;
    m_instructions[0xDD].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xDD].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0xDD].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedCompareAccumulator;
    m_instructions[0xDD].microOperationCount = 3;

    m_instructions[0xD9].operation = MOS6510Operation::CMP;
    m_instructions[0xD9].addressingMode = MOS6510AddressingMode::AbsoluteY;
    m_instructions[0xD9].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xD9].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteYAddress;
    m_instructions[0xD9].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedCompareAccumulator;
    m_instructions[0xD9].microOperationCount = 3;
    m_instructions[0xD9].pageCrossingCycle = true;

    m_instructions[0xC1].operation = MOS6510Operation::CMP;
    m_instructions[0xC1].addressingMode = MOS6510AddressingMode::IndexedIndirect;
    m_instructions[0xC1].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xC1].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xC1].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0xC1].microOperations[3] = MOS6510MicroOperation::ReadIndirectAddressHigh;
    m_instructions[0xC1].microOperations[4] = MOS6510MicroOperation::ReadIndirectCompareAccumulator;
    m_instructions[0xC1].microOperationCount = 5;

    m_instructions[0xD1].operation = MOS6510Operation::CMP;
    m_instructions[0xD1].addressingMode = MOS6510AddressingMode::IndirectIndexed;
    m_instructions[0xD1].pageCrossingCycle = true;
    m_instructions[0xD1].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xD1].microOperations[1] = MOS6510MicroOperation::ReadIndirectAddressLow;
    m_instructions[0xD1].microOperations[2] = MOS6510MicroOperation::ReadIndirectAddressHighIndexed;
    m_instructions[0xD1].microOperations[3] = MOS6510MicroOperation::ReadIndirectIndexedCompareAccumulator;
    m_instructions[0xD1].microOperationCount = 4;

    m_instructions[0xE0].operation = MOS6510Operation::CPX;
    m_instructions[0xE0].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xE0].microOperations[0] = MOS6510MicroOperation::ReadImmediateCompareXRegister;
    m_instructions[0xE0].microOperationCount = 1;

    m_instructions[0xE4].operation = MOS6510Operation::CPX;
    m_instructions[0xE4].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xE4].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xE4].microOperations[1] = MOS6510MicroOperation::ReadZeroPageCompareXRegister;
    m_instructions[0xE4].microOperationCount = 2;

    m_instructions[0xEC].operation = MOS6510Operation::CPX;
    m_instructions[0xEC].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xEC].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xEC].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xEC].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteCompareXRegister;
    m_instructions[0xEC].microOperationCount = 3;

    m_instructions[0xC0].operation = MOS6510Operation::CPY;
    m_instructions[0xC0].addressingMode = MOS6510AddressingMode::Immediate;
    m_instructions[0xC0].microOperations[0] = MOS6510MicroOperation::ReadImmediateCompareYRegister;
    m_instructions[0xC0].microOperationCount = 1;

    m_instructions[0xC4].operation = MOS6510Operation::CPY;
    m_instructions[0xC4].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xC4].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xC4].microOperations[1] = MOS6510MicroOperation::ReadZeroPageCompareYRegister;
    m_instructions[0xC4].microOperationCount = 2;

    m_instructions[0xCC].operation = MOS6510Operation::CPY;
    m_instructions[0xCC].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xCC].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xCC].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xCC].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteCompareYRegister;
    m_instructions[0xCC].microOperationCount = 3;

    m_instructions[0xE8].operation = MOS6510Operation::INX;
    m_instructions[0xE8].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xE8].microOperations[0] = MOS6510MicroOperation::IncrementXRegister;
    m_instructions[0xE8].microOperationCount = 1;

    m_instructions[0xC8].operation = MOS6510Operation::INY;
    m_instructions[0xC8].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xC8].microOperations[0] = MOS6510MicroOperation::IncrementYRegister;
    m_instructions[0xC8].microOperationCount = 1;

    m_instructions[0xCA].operation = MOS6510Operation::DEX;
    m_instructions[0xCA].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xCA].microOperations[0] = MOS6510MicroOperation::DecrementXRegister;
    m_instructions[0xCA].microOperationCount = 1;

    m_instructions[0x88].operation = MOS6510Operation::DEY;
    m_instructions[0x88].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0x88].microOperations[0] = MOS6510MicroOperation::DecrementYRegister;
    m_instructions[0x88].microOperationCount = 1;

    m_instructions[0xE6].operation = MOS6510Operation::INC;
    m_instructions[0xE6].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xE6].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xE6].microOperations[1] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xE6].microOperations[2] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xE6].microOperations[3] = MOS6510MicroOperation::IncrementDataAndWriteToMemory;
    m_instructions[0xE6].microOperationCount = 4;

    m_instructions[0xF6].operation = MOS6510Operation::INC;
    m_instructions[0xF6].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0xF6].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xF6].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xF6].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xF6].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xF6].microOperations[4] = MOS6510MicroOperation::IncrementDataAndWriteToMemory;
    m_instructions[0xF6].microOperationCount = 5;

    m_instructions[0xEE].operation = MOS6510Operation::INC;
    m_instructions[0xEE].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xEE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xEE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xEE].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xEE].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xEE].microOperations[4] = MOS6510MicroOperation::IncrementDataAndWriteToMemory;
    m_instructions[0xEE].microOperationCount = 5;

    m_instructions[0xFE].operation = MOS6510Operation::INC;
    m_instructions[0xFE].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xFE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xFE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0xFE].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0xFE].microOperations[3] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xFE].microOperations[4] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xFE].microOperations[5] = MOS6510MicroOperation::IncrementDataAndWriteToMemory;
    m_instructions[0xFE].microOperationCount = 6;

    m_instructions[0xC6].operation = MOS6510Operation::DEC;
    m_instructions[0xC6].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0xC6].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xC6].microOperations[1] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xC6].microOperations[2] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xC6].microOperations[3] = MOS6510MicroOperation::DecrementDataAndWriteToMemory;
    m_instructions[0xC6].microOperationCount = 4;

    m_instructions[0xD6].operation = MOS6510Operation::DEC;
    m_instructions[0xD6].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0xD6].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0xD6].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0xD6].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xD6].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xD6].microOperations[4] = MOS6510MicroOperation::DecrementDataAndWriteToMemory;
    m_instructions[0xD6].microOperationCount = 5;

    m_instructions[0xCE].operation = MOS6510Operation::DEC;
    m_instructions[0xCE].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0xCE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xCE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0xCE].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xCE].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xCE].microOperations[4] = MOS6510MicroOperation::DecrementDataAndWriteToMemory;
    m_instructions[0xCE].microOperationCount = 5;

    m_instructions[0xDE].operation = MOS6510Operation::DEC;
    m_instructions[0xDE].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0xDE].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0xDE].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0xDE].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0xDE].microOperations[3] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0xDE].microOperations[4] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0xDE].microOperations[5] = MOS6510MicroOperation::DecrementDataAndWriteToMemory;
    m_instructions[0xDE].microOperationCount = 6;
}

void MOS6510InstructionTable::initializeShiftInstructions()
{
    m_instructions[0x0A].operation = MOS6510Operation::ASL;
    m_instructions[0x0A].addressingMode = MOS6510AddressingMode::Accumulator;
    m_instructions[0x0A].microOperations[0] = MOS6510MicroOperation::ShiftLeftAccumulator;
    m_instructions[0x0A].microOperationCount = 1;

    m_instructions[0x06].operation = MOS6510Operation::ASL;
    m_instructions[0x06].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x06].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x06].microOperations[1] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x06].microOperations[2] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x06].microOperations[3] = MOS6510MicroOperation::ShiftLeftDataAndWriteToMemory;
    m_instructions[0x06].microOperationCount = 4;

    m_instructions[0x16].operation = MOS6510Operation::ASL;
    m_instructions[0x16].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x16].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x16].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x16].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x16].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x16].microOperations[4] = MOS6510MicroOperation::ShiftLeftDataAndWriteToMemory;
    m_instructions[0x16].microOperationCount = 5;

    m_instructions[0x0E].operation = MOS6510Operation::ASL;
    m_instructions[0x0E].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x0E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x0E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x0E].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x0E].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x0E].microOperations[4] = MOS6510MicroOperation::ShiftLeftDataAndWriteToMemory;
    m_instructions[0x0E].microOperationCount = 5;

    m_instructions[0x1E].operation = MOS6510Operation::ASL;
    m_instructions[0x1E].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x1E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x1E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x1E].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x1E].microOperations[3] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x1E].microOperations[4] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x1E].microOperations[5] = MOS6510MicroOperation::ShiftLeftDataAndWriteToMemory;
    m_instructions[0x1E].microOperationCount = 6;

    m_instructions[0x4A].operation = MOS6510Operation::LSR;
    m_instructions[0x4A].addressingMode = MOS6510AddressingMode::Accumulator;
    m_instructions[0x4A].microOperations[0] = MOS6510MicroOperation::ShiftRightAccumulator;
    m_instructions[0x4A].microOperationCount = 1;

    m_instructions[0x46].operation = MOS6510Operation::LSR;
    m_instructions[0x46].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x46].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x46].microOperations[1] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x46].microOperations[2] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x46].microOperations[3] = MOS6510MicroOperation::ShiftRightDataAndWriteToMemory;
    m_instructions[0x46].microOperationCount = 4;

    m_instructions[0x56].operation = MOS6510Operation::LSR;
    m_instructions[0x56].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x56].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x56].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x56].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x56].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x56].microOperations[4] = MOS6510MicroOperation::ShiftRightDataAndWriteToMemory;
    m_instructions[0x56].microOperationCount = 5;

    m_instructions[0x4E].operation = MOS6510Operation::LSR;
    m_instructions[0x4E].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x4E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x4E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x4E].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x4E].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x4E].microOperations[4] = MOS6510MicroOperation::ShiftRightDataAndWriteToMemory;
    m_instructions[0x4E].microOperationCount = 5;

    m_instructions[0x5E].operation = MOS6510Operation::LSR;
    m_instructions[0x5E].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x5E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x5E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x5E].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x5E].microOperations[3] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x5E].microOperations[4] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x5E].microOperations[5] = MOS6510MicroOperation::ShiftRightDataAndWriteToMemory;
    m_instructions[0x5E].microOperationCount = 6;

    m_instructions[0x2A].operation = MOS6510Operation::ROL;
    m_instructions[0x2A].addressingMode = MOS6510AddressingMode::Accumulator;
    m_instructions[0x2A].microOperations[0] = MOS6510MicroOperation::RotateLeftAccumulator;
    m_instructions[0x2A].microOperationCount = 1;

    m_instructions[0x26].operation = MOS6510Operation::ROL;
    m_instructions[0x26].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x26].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x26].microOperations[1] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x26].microOperations[2] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x26].microOperations[3] = MOS6510MicroOperation::RotateLeftDataAndWriteToMemory;
    m_instructions[0x26].microOperationCount = 4;

    m_instructions[0x36].operation = MOS6510Operation::ROL;
    m_instructions[0x36].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x36].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x36].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x36].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x36].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x36].microOperations[4] = MOS6510MicroOperation::RotateLeftDataAndWriteToMemory;
    m_instructions[0x36].microOperationCount = 5;

    m_instructions[0x2E].operation = MOS6510Operation::ROL;
    m_instructions[0x2E].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x2E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x2E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x2E].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x2E].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x2E].microOperations[4] = MOS6510MicroOperation::RotateLeftDataAndWriteToMemory;
    m_instructions[0x2E].microOperationCount = 5;

    m_instructions[0x3E].operation = MOS6510Operation::ROL;
    m_instructions[0x3E].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x3E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x3E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x3E].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x3E].microOperations[3] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x3E].microOperations[4] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x3E].microOperations[5] = MOS6510MicroOperation::RotateLeftDataAndWriteToMemory;
    m_instructions[0x3E].microOperationCount = 6;

    m_instructions[0x6A].operation = MOS6510Operation::ROR;
    m_instructions[0x6A].addressingMode = MOS6510AddressingMode::Accumulator;
    m_instructions[0x6A].microOperations[0] = MOS6510MicroOperation::RotateRightAccumulator;
    m_instructions[0x6A].microOperationCount = 1;

    m_instructions[0x66].operation = MOS6510Operation::ROR;
    m_instructions[0x66].addressingMode = MOS6510AddressingMode::ZeroPage;
    m_instructions[0x66].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x66].microOperations[1] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x66].microOperations[2] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x66].microOperations[3] = MOS6510MicroOperation::RotateRightDataAndWriteToMemory;
    m_instructions[0x66].microOperationCount = 4;

    m_instructions[0x76].operation = MOS6510Operation::ROR;
    m_instructions[0x76].addressingMode = MOS6510AddressingMode::ZeroPageX;
    m_instructions[0x76].microOperations[0] = MOS6510MicroOperation::ReadZeroPageAddress;
    m_instructions[0x76].microOperations[1] = MOS6510MicroOperation::ReadZeroPageIndexedAddress;
    m_instructions[0x76].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x76].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x76].microOperations[4] = MOS6510MicroOperation::RotateRightDataAndWriteToMemory;
    m_instructions[0x76].microOperationCount = 5;

    m_instructions[0x6E].operation = MOS6510Operation::ROR;
    m_instructions[0x6E].addressingMode = MOS6510AddressingMode::Absolute;
    m_instructions[0x6E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x6E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteAddressHigh;
    m_instructions[0x6E].microOperations[2] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x6E].microOperations[3] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x6E].microOperations[4] = MOS6510MicroOperation::RotateRightDataAndWriteToMemory;
    m_instructions[0x6E].microOperationCount = 5;

    m_instructions[0x7E].operation = MOS6510Operation::ROR;
    m_instructions[0x7E].addressingMode = MOS6510AddressingMode::AbsoluteX;
    m_instructions[0x7E].microOperations[0] = MOS6510MicroOperation::ReadAbsoluteAddressLow;
    m_instructions[0x7E].microOperations[1] = MOS6510MicroOperation::ReadAbsoluteXAddress;
    m_instructions[0x7E].microOperations[2] = MOS6510MicroOperation::ReadAbsoluteIndexedDummy;
    m_instructions[0x7E].microOperations[3] = MOS6510MicroOperation::ReadMemoryToData;
    m_instructions[0x7E].microOperations[4] = MOS6510MicroOperation::WriteDataToMemory;
    m_instructions[0x7E].microOperations[5] = MOS6510MicroOperation::RotateRightDataAndWriteToMemory;
    m_instructions[0x7E].microOperationCount = 6;
}

void MOS6510InstructionTable::initializeSpecialInstructions()
{
    m_instructions[0xEA].operation = MOS6510Operation::NOP;
    m_instructions[0xEA].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xEA].microOperations[0] = MOS6510MicroOperation::NoOperation;
    m_instructions[0xEA].microOperationCount = 1;

    m_instructions[0xF8].operation = MOS6510Operation::SED;
    m_instructions[0xF8].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xF8].microOperations[0] = MOS6510MicroOperation::SetDecimalFlag;
    m_instructions[0xF8].microOperationCount = 1;

    m_instructions[0xD8].operation = MOS6510Operation::CLD;
    m_instructions[0xD8].addressingMode = MOS6510AddressingMode::Implied;
    m_instructions[0xD8].microOperations[0] = MOS6510MicroOperation::ClearDecimalFlag;
    m_instructions[0xD8].microOperationCount = 1;

}