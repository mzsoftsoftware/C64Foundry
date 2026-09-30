#pragma once

#include <QObject>


class C64ROMSetTest : public QObject
{
    Q_OBJECT

private slots:
    void testValid();
    void testInvalidBasicROM();
    void testInvalidKernalROM();
    void testInvalidCharacterROM();

    void testROMFileNotFound();
};
