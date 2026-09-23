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
    MOS6510Instruction m_instructions[256];
};
