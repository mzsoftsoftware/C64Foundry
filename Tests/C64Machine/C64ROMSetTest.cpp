#include "C64ROMSetTest.h"

#include <QTemporaryFile>
#include <QTest>

#include "C64/C64ROMSet.h"


void C64ROMSetTest::testValid()
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

    QVERIFY(romSet.isBasicROMValid());
    QVERIFY(romSet.isKernalROMValid());
    QVERIFY(romSet.isCharacterROMValid());
    QVERIFY(romSet.isValid());
}

void C64ROMSetTest::testInvalidBasicROM()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QCOMPARE(basicFile.write(QByteArray(8191, char(0x12))), qsizetype(8191));
    QCOMPARE(kernalFile.write(QByteArray(8192, char(0x34))), qsizetype(8192));
    QCOMPARE(characterFile.write(QByteArray(4096, char(0x56))), qsizetype(4096));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet;
    romSet.basicROMFileName = basicFile.fileName();
    romSet.kernalROMFileName = kernalFile.fileName();
    romSet.characterROMFileName = characterFile.fileName();

    QVERIFY(!romSet.isBasicROMValid());
    QVERIFY(romSet.isKernalROMValid());
    QVERIFY(romSet.isCharacterROMValid());
    QVERIFY(!romSet.isValid());
}

void C64ROMSetTest::testInvalidKernalROM()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QCOMPARE(basicFile.write(QByteArray(8192, char(0x12))), qsizetype(8192));
    QCOMPARE(kernalFile.write(QByteArray(8191, char(0x34))), qsizetype(8191));
    QCOMPARE(characterFile.write(QByteArray(4096, char(0x56))), qsizetype(4096));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet;
    romSet.basicROMFileName = basicFile.fileName();
    romSet.kernalROMFileName = kernalFile.fileName();
    romSet.characterROMFileName = characterFile.fileName();

    QVERIFY(romSet.isBasicROMValid());
    QVERIFY(!romSet.isKernalROMValid());
    QVERIFY(romSet.isCharacterROMValid());
    QVERIFY(!romSet.isValid());
}

void C64ROMSetTest::testInvalidCharacterROM()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QCOMPARE(basicFile.write(QByteArray(8192, char(0x12))), qsizetype(8192));
    QCOMPARE(kernalFile.write(QByteArray(8192, char(0x34))), qsizetype(8192));
    QCOMPARE(characterFile.write(QByteArray(4095, char(0x56))), qsizetype(4095));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet;
    romSet.basicROMFileName = basicFile.fileName();
    romSet.kernalROMFileName = kernalFile.fileName();
    romSet.characterROMFileName = characterFile.fileName();

    QVERIFY(romSet.isBasicROMValid());
    QVERIFY(romSet.isKernalROMValid());
    QVERIFY(!romSet.isCharacterROMValid());
    QVERIFY(!romSet.isValid());
}

void C64ROMSetTest::testROMFileNotFound()
{
    C64ROMSet romSet;

    romSet.basicROMFileName = QStringLiteral("/this/file/does/not/exist/basic.rom");
    romSet.kernalROMFileName = QStringLiteral("/this/file/does/not/exist/kernal.rom");
    romSet.characterROMFileName = QStringLiteral("/this/file/does/not/exist/chargen.rom");

    QVERIFY(!romSet.isBasicROMValid());
    QVERIFY(!romSet.isKernalROMValid());
    QVERIFY(!romSet.isCharacterROMValid());
    QVERIFY(!romSet.isValid());
}
