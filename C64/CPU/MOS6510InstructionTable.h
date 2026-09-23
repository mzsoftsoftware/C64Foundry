#pragma once

#include <QtGlobal>

#include "MOS6510Instruction.h"


class MOS6510InstructionTable
{
public:
    explicit MOS6510InstructionTable();
    virtual ~MOS6510InstructionTable();

    const MOS6510Instruction& instruction(quint8 opcode) const     { return m_instructions[opcode];}

private:
    void initializeInstructions();

    void initializeLoadInstructions();
    void initializeStoreInstructions();
    void initializeTransferInstructions();
    void initializeStackInstructions();
    void initializeLogicalInstructions();
    void initializeArithmeticInstructions();
    void initializeSpecialInstructions();


    //void initializeCompareInstructions();
    //void initializeIncrementInstructions();
    //void initializeShiftInstructions();
    //void initializeBranchInstructions();
    //void initializeJumpInstructions();
    //void initializeFlagInstructions();

private:
    MOS6510Instruction m_instructions[256];
};
