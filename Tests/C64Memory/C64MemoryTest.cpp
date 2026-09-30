#include "C64MemoryTest.h"

#include <QTest>
#include <QTemporaryFile>

#include "C64/Memory/C64Memory.h"


void C64MemoryTest::testLoadBasicROM()
{
    C64Memory memory;

    QByteArray data(8192, char(0x00));
    data[0x0000] = char(0x12);
    data[0x1234] = char(0x34);
    data[0x1FFF] = char(0x56);

    QVERIFY(memory.loadBasicROM(data));

    QCOMPARE(memory.readBasicROM(0x0000), quint8(0x12));
    QCOMPARE(memory.readBasicROM(0x1234), quint8(0x34));
    QCOMPARE(memory.readBasicROM(0x1FFF), quint8(0x56));
}

void C64MemoryTest::testLoadKernalROM()
{
    C64Memory memory;

    QByteArray basicData(8192, char(0x12));
    QByteArray kernalData(8192, char(0x34));

    QVERIFY(memory.loadBasicROM(basicData));
    QVERIFY(memory.loadKernalROM(kernalData));

    QCOMPARE(memory.readBasicROM(0x0000), quint8(0x12));
    QCOMPARE(memory.readBasicROM(0x1FFF), quint8(0x12));

    QCOMPARE(memory.readKernalROM(0x0000), quint8(0x34));
    QCOMPARE(memory.readKernalROM(0x1FFF), quint8(0x34));
}

void C64MemoryTest::testLoadCharacterROM()
{
    C64Memory memory;

    QByteArray data(4096, char(0x00));
    data[0x0000] = char(0x12);
    data[0x0800] = char(0x34);
    data[0x0FFF] = char(0x56);

    QVERIFY(memory.loadCharacterROM(data));

    QCOMPARE(memory.readCharacterROM(0x0000), quint8(0x12));
    QCOMPARE(memory.readCharacterROM(0x0800), quint8(0x34));
    QCOMPARE(memory.readCharacterROM(0x0FFF), quint8(0x56));
}

void C64MemoryTest::testLoadBasicROMFile()
{
    QTemporaryFile file;

    QVERIFY(file.open());

    QByteArray data(8192, char(0x00));
    data[0x0000] = char(0x12);
    data[0x1234] = char(0x34);
    data[0x1FFF] = char(0x56);

    QCOMPARE(file.write(data), qsizetype(8192));
    file.close();

    C64Memory memory;

    QVERIFY(memory.loadBasicROM(file.fileName()));

    QCOMPARE(memory.readBasicROM(0x0000), quint8(0x12));
    QCOMPARE(memory.readBasicROM(0x1234), quint8(0x34));
    QCOMPARE(memory.readBasicROM(0x1FFF), quint8(0x56));
}
