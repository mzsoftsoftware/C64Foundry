#pragma once

#include <QDir>

#include "C64/C64ROMSet.h"


class ROMSetDetector
{
public:
    ROMSetDetector() = default;

    C64ROMSet detectVICE() const;

private:
    QString findROM(const QDir& directory, const QString& prefix, const QString& preferredFileName) const;
};
