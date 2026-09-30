#pragma once

#include <QObject>


class C64MemoryTest : public QObject
{
    Q_OBJECT

private slots:
    void testLoadBasicROM();
    void testLoadKernalROM();
    void testLoadCharacterROM();

    void testLoadBasicROMFile();

    void testColorRAM();
};
