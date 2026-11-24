#pragma once

#include <QMetaType>

#include "C64/C64Timing.h"


class VideoFrameSource
{
public:
    virtual ~VideoFrameSource() = default;

    virtual const C64::Timing& timing() const = 0;
    virtual const quint8* acquireVideoFrame() = 0;
};

Q_DECLARE_METATYPE(VideoFrameSource*)
