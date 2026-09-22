#pragma once

#include <QtGlobal>

namespace C64
{
    enum class VideoStandard
    {
        PAL,
        NTSC,
        NTSCOld
    };

    struct Timing
    {
        VideoStandard videoStandard;

        quint64 cyclesPerSecond;
        quint32 cyclesPerLine;
        quint32 linesPerFrame;
        quint64 cyclesPerFrame;

        quint64 viciiClock;
    };
    constexpr Timing PALTiming
    {
        VideoStandard::PAL,

        985248,
        63,
        312,
        19656,

        7881984
    };
    constexpr Timing NTSCTiming
    {
        VideoStandard::NTSC,

        1022727,
        65,
        263,
        17095,

        8181816
    };
    constexpr Timing NTSCOldTiming
    {
        VideoStandard::NTSCOld,

        1022727,
        64,
        262,
        16768,

        8181816
    };
}
