#include "EmulationScheduler.h"

#include "C64/C64Machine.h"
#include "C64/IEC/IECSystem.h"


EmulationScheduler::EmulationScheduler(C64Machine* ptrMachine, IECSystem* ptrIECSystem)
    : m_ptrMachine(ptrMachine),
    m_ptrIECSystem(ptrIECSystem)
{
}


void EmulationScheduler::clock()
{
    m_ptrMachine->clock();
    m_ptrIECSystem->clock();
}


void EmulationScheduler::runCycles(const quint64 cycles)
{
    for (quint64 cycle = 0; cycle < cycles; ++cycle)
        clock();
}
