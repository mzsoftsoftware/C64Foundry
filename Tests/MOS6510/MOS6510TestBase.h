#pragma once

#include "C64/Bus/C64Bus.h"
#include "C64/CPU/MOS6510.h"
#include "C64/Memory/C64Memory.h"


class MOS6510TestBase
{
public:
    explicit MOS6510TestBase();
    virtual ~MOS6510TestBase();

    void setupCpu();

protected:
    void clock();
    void verifyRead(quint16 address, quint8 value);
    void verifyWrite(quint16 address, quint8 value);
    void verifyNoAccess();

protected:
    C64Memory m_memory;
    C64Bus m_bus;
    MOS6510 m_cpu;
};
