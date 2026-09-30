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

void C64MachineTest::testExecuteProgram()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QByteArray kernalROM(8192, char(0x00));

    //
    // Reset vector $FFFC/$FFFD -> $0800.
    //
    kernalROM[0x1FFC] = char(0x00);
    kernalROM[0x1FFD] = char(0x08);

    QCOMPARE(basicFile.write(QByteArray(8192, char(0x00))), qsizetype(8192));
    QCOMPARE(kernalFile.write(kernalROM), qsizetype(8192));
    QCOMPARE(characterFile.write(QByteArray(4096, char(0x00))), qsizetype(4096));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet(basicFile.fileName(), kernalFile.fileName(), characterFile.fileName());
    C64Machine machine;
    QVERIFY(machine.loadROMSet(romSet));

    QByteArray program;
    program.append(char(0xA9));     // LDA #$42
    program.append(char(0x42));
    program.append(char(0x85));     // STA $02
    program.append(char(0x02));
    program.append(char(0x02));     // KIL

    QVERIFY(machine.loadProgram(0x0800, program));
    machine.powerOn();
    machine.runCycles(20);
    QCOMPARE(machine.readRAM(0x0002), quint8(0x42));
}

void C64MachineTest::testVICIIRasterIRQ()
{
    QTemporaryFile basicFile;
    QTemporaryFile kernalFile;
    QTemporaryFile characterFile;

    QVERIFY(basicFile.open());
    QVERIFY(kernalFile.open());
    QVERIFY(characterFile.open());

    QByteArray kernalROM(8192, char(0x00));

    //
    // Reset vector $FFFC/$FFFD -> $0800.
    //
    kernalROM[0x1FFC] = char(0x00);
    kernalROM[0x1FFD] = char(0x08);

    //
    // IRQ vector $FFFE/$FFFF -> $0900.
    //
    kernalROM[0x1FFE] = char(0x00);
    kernalROM[0x1FFF] = char(0x09);

    QCOMPARE(basicFile.write(QByteArray(8192, char(0x00))), qsizetype(8192));
    QCOMPARE(kernalFile.write(kernalROM), qsizetype(8192));
    QCOMPARE(characterFile.write(QByteArray(4096, char(0x00))), qsizetype(4096));

    basicFile.close();
    kernalFile.close();
    characterFile.close();

    C64ROMSet romSet(
        basicFile.fileName(),
        kernalFile.fileName(),
        characterFile.fileName());

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));

    QByteArray program;

    //
    // Enable raster IRQ at raster line 1.
    //
    program.append(char(0xA9));     // LDA #$01
    program.append(char(0x01));
    program.append(char(0x8D));     // STA $D012
    program.append(char(0x12));
    program.append(char(0xD0));

    program.append(char(0xA9));     // LDA #$01
    program.append(char(0x01));
    program.append(char(0x8D));     // STA $D01A
    program.append(char(0x1A));
    program.append(char(0xD0));

    //
    // Allow maskable interrupts.
    //
    program.append(char(0x58));     // CLI

    //
    // Wait forever for the raster IRQ.
    //
    program.append(char(0x4C));     // JMP $080B
    program.append(char(0x0B));
    program.append(char(0x08));

    QVERIFY(machine.loadProgram(0x0800, program));

    QByteArray irqHandler;

    //
    // IRQ handler writes $42 to RAM.
    //
    irqHandler.append(char(0xA9));  // LDA #$42
    irqHandler.append(char(0x42));
    irqHandler.append(char(0x85));  // STA $02
    irqHandler.append(char(0x02));

    //
    // Stop execution once the IRQ handler has run.
    //
    irqHandler.append(char(0x02));  // KIL

    QVERIFY(machine.loadProgram(0x0900, irqHandler));

    machine.powerOn();

    //
    // Enough time to reach raster line 1 and execute the IRQ handler.
    //
    machine.runCycles(200);

    QCOMPARE(machine.readRAM(0x0002), quint8(0x42));
}
