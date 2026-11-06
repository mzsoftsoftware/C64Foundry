#pragma once

#include <QtGlobal>


class C64Machine;
class IECSystem;


class EmulationScheduler
{
public:
    explicit EmulationScheduler(C64Machine* ptrMachine, IECSystem* ptrIECSystem);

    void clock();
    void runCycles(quint64 cycles);

private:
    C64Machine* m_ptrMachine = nullptr;
    IECSystem* m_ptrIECSystem = nullptr;
};
