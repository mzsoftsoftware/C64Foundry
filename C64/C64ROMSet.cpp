#include "C64ROMSet.h"

#include <QFileInfo>


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
    const QFileInfo fileInfo(basicROMFileName);
    return fileInfo.isFile() && fileInfo.size() == 8192;
}

bool C64ROMSet::isKernalROMValid() const
{
    const QFileInfo fileInfo(kernalROMFileName);
    return fileInfo.isFile() && fileInfo.size() == 8192;
}

bool C64ROMSet::isCharacterROMValid() const
{
    const QFileInfo fileInfo(characterROMFileName);
    return fileInfo.isFile() && fileInfo.size() == 4096;
}

bool C64ROMSet::isValid() const
{
    return isBasicROMValid() && isKernalROMValid() && isCharacterROMValid();
}