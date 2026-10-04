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
        quint16 borderLeft40;
        quint16 borderRight40;
        quint16 borderLeft38;
        quint16 borderRight38;

        quint16 visibleWidth;
        quint16 visibleHeight;
        quint16 visibleFirstPixel;
        quint16 visibleFirstLine;
    };
    constexpr Timing PALTiming
        {
            VideoStandard::PAL,

            985248,
            63,
            312,
            19656,

            7881984,
            124,
            444,
            131,
            435,

            403,
            284,
            76,
            16
        };
    constexpr Timing NTSCTiming
        {
            VideoStandard::NTSC,

            1022727,
            65,
            263,
            17095,

            8181816,
            132,
            452,
            139,
            443,

            418,
            235,
            77,
            41
        };
    constexpr Timing NTSCOldTiming
        {
            VideoStandard::NTSCOld,

            1022727,
            64,
            262,
            16768,

            8181816,
            124,
            444,
            131,
            435,

            411,
            234,
            76,
            41
        };
    }
