#pragma once

#include <QtGlobal>

class C64Bus;


class MOS6510
{
public:
    explicit MOS6510();
    virtual ~MOS6510();

    // Setter
    void setBus(C64Bus* ptrBus);

    // Operations
    void reset();
    void clock();

private:
    C64Bus* m_ptrBus = nullptr;

    quint16 m_programCounter = 0;
    quint8  m_accumulator = 0;
    quint8  m_x = 0;
    quint8  m_y = 0;
    quint8  m_stackPointer = 0;
    quint8  m_status = 0;

    quint8  m_opcode = 0;
    quint8  m_cycle = 0;
};
