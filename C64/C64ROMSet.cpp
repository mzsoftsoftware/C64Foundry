#include "C64ROMSet.h"

#include <QFile>

C64ROMSet::C64ROMSet(const QString& basicROMFileName,
                   const QString& kernalROMFileName,
                   const QString& characterROMFileName)
    : basicROMFileName(basicROMFileName)
    , kernalROMFileName(kernalROMFileName)
    , characterROMFileName(characterROMFileName)
{

}


bool C64ROMSet::isBasicROMValid() const
{
    QFile file(basicROMFileName);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    return file.size() == 8192;
}

bool C64ROMSet::isKernalROMValid() const
{
    QFile file(kernalROMFileName);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    return file.size() == 8192;
}

bool C64ROMSet::isCharacterROMValid() const
{
    QFile file(characterROMFileName);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    return file.size() == 4096;
}

bool C64ROMSet::isValid() const
{
    return isBasicROMValid() && isKernalROMValid() && isCharacterROMValid();
}
