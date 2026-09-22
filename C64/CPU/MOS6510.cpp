#include "MOS6510.h"


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
    m_programCounter = 0x0000;
    m_accumulator = 0x00;
    m_x = 0x00;
    m_y = 0x00;
    m_stackPointer = 0xFF;
    m_status = 0x20;
    
    m_opcode = 0x00;
    
    m_cycleState = CycleState::Reset;
    m_cycle = 0;
}

void MOS6510::clock()
{
    if (m_ptrBus == nullptr)
        return;
    
    switch (m_cycleState)
    {
    case CycleState::Reset:
        m_cycleState = CycleState::FetchOpcode;
        m_cycle = 0;
        break;
        
    case CycleState::FetchOpcode:
        //fetchOpcode();
        break;
        
    case CycleState::FetchOperand:
        //fetchOperand();
        break;
        
    case CycleState::Execute:
        // Wird als Nächstes durch die eigentliche
        // zyklusweise Instruktionsausführung ersetzt.
        m_cycleState = CycleState::FetchOpcode;
        m_cycle = 0;
        break;
    }
}
