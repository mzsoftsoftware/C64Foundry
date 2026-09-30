#include "C64MachineTest.h"

#include <QTest>
#include <QTemporaryFile>

#include "C64/C64Machine.h"
#include "C64/C64ROMSet.h"


void C64MachineTest::testLoadROMSet()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QCOMPARE(basicFile.write(QByteArray(8192, char(0x12))), qsizetype(8192));
    QCOMPARE(kernalFile.write(QByteArray(8192, char(0x34))), qsizetype(8192));
    QCOMPARE(characterFile.write(QByteArray(4096, char(0x56))), qsizetype(4096));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet;
    romSet.basicROMFileName = basicFile.fileName();
    romSet.kernalROMFileName = kernalFile.fileName();
    romSet.characterROMFileName = characterFile.fileName();

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));
}

void C64MachineTest::testLoadInvalidROMSet()
{
    C64ROMSet romSet;

    C64Machine machine;

    QVERIFY(!machine.loadROMSet(romSet));
}
