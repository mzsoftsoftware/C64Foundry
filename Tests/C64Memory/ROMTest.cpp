#include "ROMTest.h"

#include <QTest>
#include <QTemporaryFile>

#include "C64/Memory/ROM.h"


ROMTest::ROMTest()
{
}
ROMTest::~ROMTest()
{
}

void ROMTest::testLoadData()
{
    ROM rom(4);

    QByteArray data;
    data.append(char(0x12));
    data.append(char(0x34));
    data.append(char(0x56));
    data.append(char(0x78));

    QVERIFY(rom.load(data));
    QCOMPARE(rom.read(0), quint8(0x12));
    QCOMPARE(rom.read(1), quint8(0x34));
    QCOMPARE(rom.read(2), quint8(0x56));
    QCOMPARE(rom.read(3), quint8(0x78));
}

void ROMTest::testLoadWrongSize()
{
    ROM rom(4);

    QByteArray data(3, char(0x12));
    QVERIFY(!rom.load(data));
}

void ROMTest::testLoadWrongSizeKeepsData()
{
    ROM rom(4);

    QByteArray validData;
    validData.append(char(0x12));
    validData.append(char(0x34));
    validData.append(char(0x56));
    validData.append(char(0x78));

    QVERIFY(rom.load(validData));

    QByteArray invalidData(3, char(0xAA));

    QVERIFY(!rom.load(invalidData));

    QCOMPARE(rom.read(0), quint8(0x12));
    QCOMPARE(rom.read(1), quint8(0x34));
    QCOMPARE(rom.read(2), quint8(0x56));
    QCOMPARE(rom.read(3), quint8(0x78));
}

void ROMTest::testLoadFile()
{
    QTemporaryFile file;

    QVERIFY(file.open());

    QByteArray data;
    data.append(char(0x12));
    data.append(char(0x34));
    data.append(char(0x56));
    data.append(char(0x78));

    QCOMPARE(file.write(data), qsizetype(4));
    file.close();

    ROM rom(4);

    QVERIFY(rom.load(file.fileName()));

    QCOMPARE(rom.read(0), quint8(0x12));
    QCOMPARE(rom.read(1), quint8(0x34));
    QCOMPARE(rom.read(2), quint8(0x56));
    QCOMPARE(rom.read(3), quint8(0x78));
}

void ROMTest::testLoadFileNotFound()
{
    ROM rom(4);
    QVERIFY(!rom.load(QStringLiteral("/this/file/does/not/exist.rom")));
}

void ROMTest::testLoadFileWrongSize()
{
    QTemporaryFile file;

    QVERIFY(file.open());

    const QByteArray data(3, char(0x12));

    QCOMPARE(file.write(data), qsizetype(3));
    file.close();

    ROM rom(4);

    QVERIFY(!rom.load(file.fileName()));
}
