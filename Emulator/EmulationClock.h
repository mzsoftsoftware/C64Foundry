#pragma once

#include <QtGlobal>

#include "EmulationTime.h"


class EmulationClock
{
public:
    explicit EmulationClock(quint64 frequency);

    void reset();

    bool setFrequency(quint64 frequency);
    bool advance(quint128 cycles);

    quint64 frequency() const;
    quint128 cycles() const;

    EmulationTime time() const;

private:
    bool calculateTime(quint128 cycles, EmulationTime& time) const;

private:
    quint64 m_frequency = 0;
    quint128 m_cycles = 0;
};
