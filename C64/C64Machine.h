#pragma once

#include <QtGlobal>

class C64Memory;
class C64Bus;
class MOS6510;

#include "C64/C64Timing.h"


class C64Machine
{
public:
    explicit C64Machine();
    virtual ~C64Machine();

    // Operations
    void powerOn();
    void reset();

    void clock();
    void runCycles(quint64 cycles);

    // Setter
    void setTiming(const C64::Timing& timing)   { m_timing = timing; }
    const C64::Timing& timing() const           { return m_timing; }

    // Getter
    quint64 cycles() const          { return m_cycles; }
    quint64 cyclesPerFrame() const  { return m_timing.cyclesPerFrame; }
    quint64 cyclesPerLine() const   { return m_timing.cyclesPerLine;  }

private:
    C64::Timing m_timing = C64::PALTiming;
    quint64 m_cycles = 0;

    C64Memory*  m_ptrMemory = nullptr;
    C64Bus*     m_ptrBus = nullptr;

    MOS6510*    m_ptrCpu = nullptr;
    // VIC-II

    // CIA-I
    // CIA-II

    // SID
};
