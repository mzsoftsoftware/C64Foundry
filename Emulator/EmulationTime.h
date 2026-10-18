#pragma once

#include <QtGlobal>


class EmulationTime
{
public:
    using Value = quint128;

    static constexpr Value PicosecondsPerSecond = 1000000000000ULL;

    constexpr EmulationTime() = default;
    explicit constexpr EmulationTime(Value picoseconds)
        : m_picoseconds(picoseconds)
    {
    }

    constexpr Value picoseconds() const
    {
        return m_picoseconds;
    }

    constexpr bool operator==(const EmulationTime& other) const
    {
        return m_picoseconds == other.m_picoseconds;
    }

    constexpr bool operator!=(const EmulationTime& other) const
    {
        return m_picoseconds != other.m_picoseconds;
    }

    constexpr bool operator<(const EmulationTime& other) const
    {
        return m_picoseconds < other.m_picoseconds;
    }

    constexpr bool operator<=(const EmulationTime& other) const
    {
        return m_picoseconds <= other.m_picoseconds;
    }

    constexpr bool operator>(const EmulationTime& other) const
    {
        return m_picoseconds > other.m_picoseconds;
    }

    constexpr bool operator>=(const EmulationTime& other) const
    {
        return m_picoseconds >= other.m_picoseconds;
    }

private:
    Value m_picoseconds = 0;
};
