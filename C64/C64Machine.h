#pragma once

#include <QtGlobal>

class C64Memory;
class C64Bus;
class MOS6510;


class C64Machine
{
public:
    explicit C64Machine();
    virtual ~C64Machine();

    void powerOn();
    void reset();

    void clock();
    void runCycles(quint64 cycles);
    void runFrame();

private:
    C64Memory*  m_ptrMemory = nullptr;
    C64Bus*     m_ptrBus = nullptr;

    MOS6510*    m_ptrCpu = nullptr;
    // VIC-II

    // CIA-I
    // CIA-II

    // SID
};
