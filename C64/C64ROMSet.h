#pragma once

#include <QString>


class C64ROMSet
{
public:
    // Validation
    bool isBasicROMValid() const;
    bool isKernalROMValid() const;
    bool isCharacterROMValid() const;
    bool isValid() const;

    QString basicROMFileName;
    QString kernalROMFileName;
    QString characterROMFileName;
};