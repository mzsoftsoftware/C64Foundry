#include "EmulationClock.h"


EmulationClock::EmulationClock(const quint64 frequency)
{
    setFrequency(frequency);
}

void EmulationClock::reset()
{
    m_cycles = 0;
}

bool EmulationClock::setFrequency(const quint64 frequency)
{
    if (frequency == 0)
        return false;

    if (m_cycles != 0)
        return false;

    m_frequency = frequency;

    return true;
}

bool EmulationClock::advance(const quint128 cycles)
{
    if (m_frequency == 0)
        return false;

    if (cycles > Q_UINT128_MAX - m_cycles)
        return false;

    const quint128 newCycles = m_cycles + cycles;

    EmulationTime newTime;

    if (!calculateTime(newCycles, newTime))
        return false;

    m_cycles = newCycles;

    return true;
}

quint64 EmulationClock::frequency() const
{
    return m_frequency;
}

quint128 EmulationClock::cycles() const
{
    return m_cycles;
}

EmulationTime EmulationClock::time() const
{
    EmulationTime result;

    calculateTime(m_cycles, result);

    return result;
}

bool EmulationClock::calculateTime(const quint128 cycles, EmulationTime& time) const
{
    if (m_frequency == 0)
        return false;

    const quint128 picosecondsPerSecond = EmulationTime::PicosecondsPerSecond;

    const quint128 seconds = cycles / m_frequency;
    const quint128 remainingCycles = cycles % m_frequency;

    // Ganze Sekunden dürfen bei der Umrechnung nicht überlaufen.
    if (seconds > Q_UINT128_MAX / picosecondsPerSecond)
        return false;

    const quint128 wholePicoseconds = seconds * picosecondsPerSecond;

    // Auch die Multiplikation des Restes muss überlaufsicher sein.
    if (remainingCycles > Q_UINT128_MAX / picosecondsPerSecond)
        return false;

    const quint128 remainingPicoseconds = (remainingCycles * picosecondsPerSecond) / m_frequency;

    // Addition beider Zeitanteile prüfen.
    if (remainingPicoseconds > Q_UINT128_MAX - wholePicoseconds)
        return false;

    time = EmulationTime(wholePicoseconds + remainingPicoseconds);

    return true;
}
