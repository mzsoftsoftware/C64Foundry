#pragma once

#include <QString>


class C64ROMSet
{
public:
    explicit C64ROMSet() = default;
    explicit C64ROMSet(const QString& basicROMFileName,
                       const QString& kernalROMFileName,
                       const QString& characterROMFileName);

    // Validation
    bool isBasicROMValid() const;
    bool isKernalROMValid() const;
    bool isCharacterROMValid() const;
    bool isValid() const;

    QString basicROMFileName;
    QString kernalROMFileName;
    QString characterROMFileName;
};
