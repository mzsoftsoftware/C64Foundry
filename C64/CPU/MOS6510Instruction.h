#pragma once

#include <QtGlobal>

#include "MOS6510MicroOperation.h"

enum class MOS6510Operation
{
    ADC,
    ANC,
    AND,
    ARR,
    ASL,
    ASR,
    BCC,
    BCS,
    BEQ,
    BIT,
    BMI,
    BNE,
    BPL,
    BRK,
    BVC,
    BVS,

    CLC,
    CLD,
    CLI,
    CLV,

    CMP,
    CPX,
    CPY,

    DCP,
    DEC,
    DEX,
    DEY,

    EOR,

    INC,
    INX,
    INY,

    ISB,

    JMP,
    JSR,

    LAS,
    LAX,

    LDA,
    LDX,
    LDY,

    LSR,

    NOP,

    ORA,

    PHA,
    PHP,
    PLA,
    PLP,

    RLA,
    ROL,
    ROR,
    RRA,
    RTI,
    RTS,

    SAX,
    SBC,
    SEC,
    SED,
    SEI,
    SLO,

    STA,
    STX,
    STY,

    TAX,
    TAY,
    TSX,
    TXA,
    TXS,
    TYA,

    Unknown
};

enum class MOS6510AddressingMode
{
    Implied,
    Accumulator,
    Immediate,

    ZeroPage,
    ZeroPageX,
    ZeroPageY,

    Absolute,
    AbsoluteX,
    AbsoluteY,

    Indirect,
    IndexedIndirect,
    IndirectIndexed,

    Relative
};

struct MOS6510Instruction
{
    MOS6510Operation operation;
    MOS6510AddressingMode addressingMode;

    MOS6510MicroOperation microOperations[8];
    quint8 microOperationCount;

    bool pageCrossingCycle = false;
};
