#include "C64MachineTest.h"

#include <QTest>
#include <QTemporaryFile>
#include <QByteArray>

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

void C64MachineTest::testRunCycles()
{
    C64Machine machine;
    QCOMPARE(machine.cycles(), quint64(0));
    machine.runCycles(100);
    QCOMPARE(machine.cycles(), quint64(100));
    machine.runCycles(23);
    QCOMPARE(machine.cycles(), quint64(123));
}

void C64MachineTest::testLoadProgram()
{
    C64Machine machine;

    QByteArray program;
    program.append(char(0xA9));     // LDA #$42
    program.append(char(0x42));
    program.append(char(0x85));     // STA $02
    program.append(char(0x02));
    program.append(char(0x02));     // KIL

    QVERIFY(machine.loadProgram(0x0800, program));

    //
    // A program must not wrap around the end of RAM.
    //
    QByteArray overflowProgram;
    overflowProgram.append(char(0x01));
    overflowProgram.append(char(0x02));
    overflowProgram.append(char(0x03));

    QVERIFY(!machine.loadProgram(0xFFFE, overflowProgram));

    //
    // Ending exactly at $FFFF is valid.
    //
    QByteArray endOfMemoryProgram;
    endOfMemoryProgram.append(char(0x01));
    endOfMemoryProgram.append(char(0x02));

    QVERIFY(machine.loadProgram(0xFFFE, endOfMemoryProgram));
}
