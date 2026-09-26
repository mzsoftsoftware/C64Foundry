#pragma once

#include <QtGlobal>

#include "MOS6510Instruction.h"
#include "MOS6510InstructionTable.h"
#include "MOS6510MicroOperation.h"
#include "MOS6510StatusRegister.h"


class C64Bus;


class MOS6510
{
public:
    explicit MOS6510();
    virtual ~MOS6510();

    // Getter
    quint8 accumulator() const                          { return m_accumulator; }
    quint8 xRegister() const                            { return m_x; }
    quint8 yRegister() const                            { return m_y; }
    quint8 stackPointer() const                         { return m_stackPointer; }
    quint16 programCounter() const                      { return m_programCounter; }
    quint8 status() const                               { return m_status; }
    bool statusFlag(MOS6510StatusFlag flag) const;
    bool irqLine() const                                { return m_irqLine; }
    bool nmiLine() const                                { return m_nmiLine; }

    // Setter
    void setBus(C64Bus* ptrBus);
    void setAccumulator(quint8 value)                   { m_accumulator = value; }
    void setXRegister(quint8 value)                     { m_x = value; }
    void setYRegister(quint8 value)                     { m_y = value; }
    void setStackPointer(quint8 value)                  { m_stackPointer = value; }
    void setProgramCounter(quint16 address)             { m_programCounter = address; }
    void setStatus(quint8 value)                        { m_status = value; }
    void setStatusFlag(MOS6510StatusFlag flag, bool value);
    void setIrqLine(bool active)                        { m_irqLine = active; }
    void setNmiLine(bool active);

    // Operations
    void initialize();
    void reset();
    void clock();

private:
    enum class CpuState
    {
        Fetch,
        Execute,
        Reset,
        Irq,
        Nmi,
        Stopped
    };

    void executeResetCycle();
    void executeIrqCycle();
    void pollIrq();
    void executeNmiCycle();
    void pollNmi();

    void executeMicroOperation(MOS6510MicroOperation microOperation);

    void updateLoadFlags(quint8 value);

    void addToAccumulator(quint8 operand);
    void subtractFromAccumulator(quint8 operand);

private:
    C64Bus* m_ptrBus = nullptr;

    MOS6510InstructionTable m_instructionTable;
    CpuState m_state = CpuState::Fetch;
    quint8 m_resetCycle = 0;
    bool m_irqLine = false;
    bool m_irqPending = false;
    bool m_irqPolled = false;
    bool m_initialFetch = true;
    quint8 m_irqCycle = 0;
    bool m_nmiLine = false;
    bool m_nmiPending = false;
    bool m_nmiAccepted = false;
    bool m_nmiDelay = false;
    bool m_nmiVectorFetch = false;
    bool m_nmiHijack = false;
    quint8 m_nmiCycle = 0;

    quint8  m_accumulator = 0;
    quint8  m_x = 0;
    quint8  m_y = 0;
    quint8  m_stackPointer = 0;
    quint16 m_programCounter = 0;
    quint8  m_status = 0;

    quint8 m_opcode = 0;
    quint16 m_address = 0;
    quint8 m_data = 0;

    MOS6510Operation m_operation = MOS6510Operation::Unknown;
    MOS6510AddressingMode m_addressingMode = MOS6510AddressingMode::Implied;
    const MOS6510Instruction* m_ptrInstruction = nullptr;
    quint8 m_microOperationCount = 0;
    quint8 m_microOperationIndex = 0;

    bool m_pageCrossed = false;
    bool m_dummyReadPending = false;
    bool m_pageCrossingCycle = false;
};
