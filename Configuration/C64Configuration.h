#pragma once

#include <QString>

#include "C64/C64ROMSet.h"


class C64Configuration
{
public:
    C64Configuration() = default;

    QString name;
    C64ROMSet romSet;
};