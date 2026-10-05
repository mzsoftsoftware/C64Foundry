#pragma once

#include <QtGlobal>

class C64ROMSet;
class C64Memory;
class C64Bus;
class MOS6510;
class VICII;
class MOS6526;

#include "C64/C64Timing.h"


class C64Machine
{
public:
    explicit C64Machine();
    virtual ~C64Machine();

    // Operations
    bool loadROMSet(const C64ROMSet& romSet);
    bool loadProgram(quint16 address, const QByteArray& data);

    void powerOn();
    void reset();

    void clock();
    void runCycles(quint64 cycles);

    // Setter
    void setTiming(const C64::Timing& timing);
    void writeVICIIRegister(quint8 address, quint8 value);
    void writeCIA1Register(quint8 address, quint8 value);
    void writeCIA2Register(quint8 address, quint8 value);

    // Getter
    const C64::Timing& timing() const           { return m_timing; }
    quint8 readRAM(quint16 address) const;

    bool viciiBA() const;
    bool viciiAEC() const;

    quint8 readCIA1Register(quint8 address) const;
    quint8 readCIA2Register(quint8 address) const;

    bool busAEC() const;
    quint8 busAccessCount() const;
    bool busLastAccessWasRead() const;
    bool busLastAccessWasWrite() const;
    quint16 busLastAccessAddress() const;
    quint8 busLastAccessValue() const;

    quint64 cycles() const          { return m_cycles; }
    quint64 cyclesPerFrame() const  { return m_timing.cyclesPerFrame; }
    quint64 cyclesPerLine() const   { return m_timing.cyclesPerLine;  }

    const quint8* acquireVideoFrame();
    const quint8* peekVideoFrame() const;
    quint16 videoFrameWidth() const;
    quint16 videoFrameHeight() const;

private:
    C64::Timing m_timing = C64::PALTiming;
    quint64 m_cycles = 0;

    C64Memory*  m_ptrMemory = nullptr;
    C64Bus*     m_ptrBus = nullptr;

    MOS6510*    m_ptrCpu = nullptr;
    VICII*      m_ptrVICII = nullptr;

    MOS6526*    m_ptrCIA1 = nullptr;
    MOS6526*    m_ptrCIA2 = nullptr;

    // SID
};
