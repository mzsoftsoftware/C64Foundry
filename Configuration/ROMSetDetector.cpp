#include "ROMSetDetector.h"


C64ROMSet ROMSetDetector::detectVICE() const
{
    const QDir directory(QStringLiteral("/usr/share/vice/C64"));
    C64ROMSet romSet;

    if (!directory.exists())
        return romSet;

    romSet.basicROMFileName = findROM(directory, QStringLiteral("basic"), QStringLiteral("basic-901226-01.bin"));
    romSet.kernalROMFileName = findROM(directory, QStringLiteral("kernal"), QStringLiteral("kernal-901227-03.bin"));
    romSet.characterROMFileName = findROM(directory, QStringLiteral("chargen"), QStringLiteral("chargen-901225-01.bin"));
    if (romSet.basicROMFileName.isEmpty() ||
        romSet.kernalROMFileName.isEmpty() ||
        romSet.characterROMFileName.isEmpty())
    {
        return C64ROMSet();
    }

    return romSet;
}

QString ROMSetDetector::findROM(const QDir& directory, const QString& prefix, const QString& preferredFileName) const
{
    if (directory.exists(preferredFileName))
        return directory.filePath(preferredFileName);

    const QStringList files = directory.entryList({ prefix + QStringLiteral("-*.bin") }, QDir::Files, QDir::Name);
    if (files.isEmpty())
        return {};

    return directory.filePath(files.first());
}