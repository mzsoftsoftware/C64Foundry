#include "C64MachineTest.h"

#include <QTest>
#include <QTemporaryFile>
#include <QByteArray>
#include <QElapsedTimer>

#include <algorithm>
#include <limits>

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
void C64MachineTest::testVICIIAEC()
{
    C64Machine machine;

    //
    // Enable display and select YSCROLL 0.
    // Raster line $30 is therefore a badline.
    //
    machine.writeVICIIRegister(0x11, 0x10);

    //
    // Advance to raster line $30, cycle 14.
    //
    machine.runCycles(
        0x30 * C64::PALTiming.cyclesPerLine + 14);

    //
    // At cycle 14, BA has already been low for two cycles,
    // but AEC is still high.
    //
    QVERIFY(!machine.viciiBA());
    QVERIFY(machine.viciiAEC());
    QVERIFY(machine.busAEC());

    //
    // Cycle 15 is the first c-access.
    // BA remains low and AEC goes low.
    //
    machine.clock();

    QVERIFY(!machine.viciiBA());
    QVERIFY(!machine.viciiAEC());
    QVERIFY(!machine.busAEC());

    //
    // AEC remains low through cycle 54.
    //
    machine.runCycles(39);

    QVERIFY(!machine.viciiBA());
    QVERIFY(!machine.viciiAEC());
    QVERIFY(!machine.busAEC());

    //
    // AEC returns high at cycle 55.
    // BA returns high as well.
    //
    machine.clock();

    QVERIFY(machine.viciiBA());
    QVERIFY(machine.viciiAEC());
    QVERIFY(machine.busAEC());
}
void C64MachineTest::testVICIIBAWrite()
{
    C64Machine machine;

    //
    // Enable display and select YSCROLL 0.
    // Raster line $30 is therefore a badline.
    //
    machine.writeVICIIRegister(0x11, 0x10);

    //
    // Execute a simple program while the VIC-II advances
    // towards the first badline.
    //
    QByteArray program;

    program.append(char(0xEA));     // NOP

    program.append(char(0xA9));     // LDA #$42
    program.append(char(0x42));

    program.append(char(0x85));     // STA $02
    program.append(char(0x02));

    program.append(char(0x4C));     // JMP $0803
    program.append(char(0x03));
    program.append(char(0x08));

    QVERIFY(machine.loadProgram(0x0800, program));

    machine.powerOn();

    //
    // Advance to raster line $30, cycle 11.
    //
    machine.runCycles(
        0x30 * C64::PALTiming.cyclesPerLine + 11);

    QVERIFY(machine.viciiBA());
    QVERIFY(machine.viciiAEC());

    //
    // At cycle 12, BA goes low while AEC remains high.
    // The CPU is performing a write cycle, which must
    // therefore still reach the system bus.
    //
    machine.clock();

    QVERIFY(!machine.viciiBA());
    QVERIFY(machine.viciiAEC());

    QCOMPARE(machine.busAccessCount(), quint8(1));
    QVERIFY(machine.busLastAccessWasWrite());
    QCOMPARE(machine.busLastAccessAddress(), quint16(0x01EA));
    QCOMPARE(machine.busLastAccessValue(), quint8(0x34));
    QCOMPARE(machine.readRAM(0x01EA), quint8(0x34));
}
void C64MachineTest::testVICIIAECStopsCPUAccess()
{
    C64Machine machine;

    //
    // Enable display and select YSCROLL 0.
    // Raster line $30 is therefore a badline.
    //
    machine.writeVICIIRegister(0x11, 0x10);

    //
    // Advance to raster line $30, cycle 14.
    //
    machine.runCycles(
        0x30 * C64::PALTiming.cyclesPerLine + 14);

    QVERIFY(!machine.viciiBA());
    QVERIFY(machine.viciiAEC());
    QVERIFY(machine.busAEC());

    //
    // Cycle 15 is the first c-access.
    // AEC goes low and disconnects the CPU from the bus.
    //
    machine.clock();

    QVERIFY(!machine.viciiBA());
    QVERIFY(!machine.viciiAEC());
    QVERIFY(!machine.busAEC());

    //
    // The VIC-II performs the c-access in this cycle.
    // Therefore exactly one bus access must be visible,
    // and it must not be a CPU access.
    //
    QCOMPARE(machine.busAccessCount(), quint8(1));
    QVERIFY(machine.busLastAccessWasRead());
    QVERIFY(machine.busLastAccessWasVICII());
}
void C64MachineTest::testVICIIBadLineCPUStall()
{
    C64Machine machine;

    //
    // Enable display and select YSCROLL 0.
    // Raster line $30 is therefore a badline.
    //
    machine.writeVICIIRegister(0x11, 0x10);

    //
    // Advance to raster line $30, cycle 14.
    // BA is already low, AEC is still high.
    //
    machine.runCycles(
        0x30 * C64::PALTiming.cyclesPerLine + 14);

    QVERIFY(!machine.viciiBA());
    QVERIFY(machine.viciiAEC());

    const quint64 cpuCyclesBefore =
        machine.cpuCycles();

    const quint16 programCounterBefore =
        machine.cpuProgramCounter();

    //
    // Cycles 15 through 54 belong to the VIC-II.
    // The CPU is stalled on its current read cycle.
    //
    machine.runCycles(40);

    QCOMPARE(machine.cpuCycles(), cpuCyclesBefore);
    QCOMPARE(machine.cpuProgramCounter(), programCounterBefore);

    QVERIFY(!machine.viciiBA());
    QVERIFY(!machine.viciiAEC());

    //
    // Cycle 55 releases the bus.
    // The stalled CPU cycle must now continue.
    //
    machine.clock();

    QVERIFY(machine.viciiBA());
    QVERIFY(machine.viciiAEC());

    QCOMPARE(machine.cpuCycles(), cpuCyclesBefore + 1);
}

void C64MachineTest::testCIARegisterAccess()
{
    C64Machine machine;

    //
    // Configure Port A of both CIAs as outputs.
    //
    machine.writeCIA1Register(0x02, 0xFF);
    machine.writeCIA2Register(0x02, 0xFF);

    //
    // Write different values to verify that CIA 1 and
    // CIA 2 are independent devices.
    //
    machine.writeCIA1Register(0x00, 0x5A);
    machine.writeCIA2Register(0x00, 0xA5);
    QCOMPARE(machine.readCIA1Register(0x00), quint8(0x5A));
    QCOMPARE(machine.readCIA2Register(0x00), quint8(0xA5));
}
void C64MachineTest::testCIAClock()
{
    C64Machine machine;

    //
    // Load different values into Timer A of both CIAs.
    //
    machine.writeCIA1Register(0x04, 0x02);
    machine.writeCIA1Register(0x05, 0x00);

    machine.writeCIA2Register(0x04, 0x03);
    machine.writeCIA2Register(0x05, 0x00);

    //
    // Start Timer A of both CIAs.
    //
    machine.writeCIA1Register(0x0E, 0x01);
    machine.writeCIA2Register(0x0E, 0x01);

    QCOMPARE(machine.readCIA1Register(0x04), quint8(0x02));
    QCOMPARE(machine.readCIA2Register(0x04), quint8(0x03));

    //
    // One C64 machine cycle must clock both CIAs once.
    //
    machine.clock();

    QCOMPARE(machine.readCIA1Register(0x04), quint8(0x01));
    QCOMPARE(machine.readCIA2Register(0x04), quint8(0x02));
}
void C64MachineTest::testCIA1IRQ()
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

    QCOMPARE(
        basicFile.write(QByteArray(8192, char(0x00))),
        qsizetype(8192));

    QCOMPARE(
        kernalFile.write(kernalROM),
        qsizetype(8192));

    QCOMPARE(
        characterFile.write(QByteArray(4096, char(0x00))),
        qsizetype(4096));

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
    // Allow maskable interrupts.
    //
    program.append(char(0x58));     // CLI

    //
    // Wait forever for the CIA1 IRQ.
    //
    program.append(char(0x4C));     // JMP $0801
    program.append(char(0x01));
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

    //
    // Enable Timer A interrupts in CIA1.
    //
    machine.writeCIA1Register(0x0D, 0x81);

    //
    // Load Timer A with a short interval.
    //
    machine.writeCIA1Register(0x04, 0x10);
    machine.writeCIA1Register(0x05, 0x00);

    //
    // Force load and start Timer A.
    //
    machine.writeCIA1Register(0x0E, 0x11);

    machine.powerOn();

    //
    // Enough time for Timer A to underflow and
    // the CPU to execute the IRQ handler.
    //
    machine.runCycles(100);

    QCOMPARE(machine.readRAM(0x0002), quint8(0x42));
}

void C64MachineTest::testROMBootScreenRAMStable()
{
    C64ROMSet romSet(
        QStringLiteral(":/ROMs/OpenROMs/basic.rom"),
        QStringLiteral(":/ROMs/OpenROMs/kernal.rom"),
        QStringLiteral(":/ROMs/OpenROMs/chargen.rom"));

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));

    machine.powerOn();

    //
    // Give the ROM enough time to complete its initialization.
    //
    machine.runCycles(
        C64::PALTiming.cyclesPerFrame * 10);

    QByteArray firstSnapshot;
    firstSnapshot.reserve(1000);

    for (quint16 address = 0x0400;
         address <= 0x07E7;
         ++address)
    {
        firstSnapshot.append(
            static_cast<char>(
                machine.readRAM(address)));
    }

    //
    // Let the initialized machine run for a longer period.
    //
    machine.runCycles(
        C64::PALTiming.cyclesPerFrame * 100);

    QByteArray secondSnapshot;
    secondSnapshot.reserve(1000);

    for (quint16 address = 0x0400;
         address <= 0x07E7;
         ++address)
    {
        secondSnapshot.append(
            static_cast<char>(
                machine.readRAM(address)));
    }

    //
    // The screen contents must remain stable.
    //
    // The cursor position is the only exception:
    // the KERNAL toggles bit 7 of the space character
    // to display the blinking reverse-space cursor.
    //
    constexpr quint16 CursorAddress = 0x05B8;

    for (qsizetype index = 0;
         index < firstSnapshot.size();
         ++index)
    {
        const quint16 address =
            static_cast<quint16>(
                0x0400 + index);

        const quint8 first =
            static_cast<quint8>(
                firstSnapshot.at(index));

        const quint8 second =
            static_cast<quint8>(
                secondSnapshot.at(index));

        if (address == CursorAddress)
        {
            //
            // The character itself must remain unchanged.
            // Only the reverse bit may differ.
            //
            QCOMPARE(
                first & 0x7F,
                second & 0x7F);

            QCOMPARE(
                first & 0x7F,
                quint8(0x20));

            QVERIFY(
                second == 0x20 ||
                second == 0xA0);
        }
        else
        {
            QCOMPARE(second, first);
        }
    }
}
void C64MachineTest::testROMBootColorRAMStable()
{
    C64ROMSet romSet(
        QStringLiteral(":/ROMs/OpenROMs/basic.rom"),
        QStringLiteral(":/ROMs/OpenROMs/kernal.rom"),
        QStringLiteral(":/ROMs/OpenROMs/chargen.rom"));

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));

    machine.powerOn();

    //
    // Give the ROM enough time to complete its initialization.
    //
    machine.runCycles(
        C64::PALTiming.cyclesPerFrame * 10);

    QByteArray firstSnapshot;
    firstSnapshot.reserve(1000);

    for (quint16 address = 0;
         address < 1000;
         ++address)
    {
        firstSnapshot.append(
            static_cast<char>(
                machine.readColorRAM(address)));
    }

    //
    // Let the initialized machine run for several more frames.
    //
    machine.runCycles(
        C64::PALTiming.cyclesPerFrame * 100);

    QByteArray secondSnapshot;
    secondSnapshot.reserve(1000);

    for (quint16 address = 0;
         address < 1000;
         ++address)
    {
        secondSnapshot.append(
            static_cast<char>(
                machine.readColorRAM(address)));
    }

    QCOMPARE(secondSnapshot, firstSnapshot);
}
void C64MachineTest::testROMBootCursorBlink()
{
    C64ROMSet romSet(
        QStringLiteral(":/ROMs/OpenROMs/basic.rom"),
        QStringLiteral(":/ROMs/OpenROMs/kernal.rom"),
        QStringLiteral(":/ROMs/OpenROMs/chargen.rom"));

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));

    machine.powerOn();

    //
    // Give the ROM enough time to complete its initialization.
    //
    machine.runCycles(
        C64::PALTiming.cyclesPerFrame * 10);

    constexpr quint16 CursorAddress = 0x05B8;

    //
    // The cursor is located on a space character.
    // Bit 7 is used to display the blinking reverse-space cursor.
    //
    const quint8 initialValue =
        machine.readRAM(CursorAddress);

    QCOMPARE(
        initialValue & 0x7F,
        quint8(0x20));

    bool sawNormal = (initialValue == 0x20);
    bool sawReverse = (initialValue == 0xA0);

    //
    // Observe the cursor for several seconds.
    //
    for (quint16 frame = 0;
         frame < 200;
         ++frame)
    {
        machine.runCycles(
            C64::PALTiming.cyclesPerFrame);

        const quint8 value =
            machine.readRAM(CursorAddress);

        //
        // The character itself must remain a space.
        // Only the reverse bit may change.
        //
        QCOMPARE(
            value & 0x7F,
            quint8(0x20));

        QVERIFY(
            value == 0x20 ||
            value == 0xA0);

        if (value == 0x20)
            sawNormal = true;

        if (value == 0xA0)
            sawReverse = true;
    }

    //
    // During the observation period the cursor must have
    // appeared in both states.
    //
    QVERIFY(sawNormal);
    QVERIFY(sawReverse);
}
void C64MachineTest::testROMBootVICIICAccess()
{
    C64ROMSet romSet(
        QStringLiteral(":/ROMs/OpenROMs/basic.rom"),
        QStringLiteral(":/ROMs/OpenROMs/kernal.rom"),
        QStringLiteral(":/ROMs/OpenROMs/chargen.rom"));

    C64Machine machine;

    QVERIFY(machine.loadROMSet(romSet));

    machine.powerOn();

    //
    // Give the ROM enough time to complete its initialization.
    //
    machine.runCycles(
        C64::PALTiming.cyclesPerFrame * 10);

    //
    // Wait for the beginning of a new VIC-II frame.
    //
    do
    {
        machine.clock();
    }
    while (machine.viciiRasterLine() != 0 ||
           machine.viciiRasterCycle() != 0);

    //
    // The first text row starts with the badline at raster $33
    // for the normal KERNAL YSCROLL setting of 3.
    //
    constexpr quint16 RasterLine = 0x33;

    //
    // Stop immediately before the first c-access.
    //
    while (machine.viciiRasterLine() != RasterLine ||
           machine.viciiRasterCycle() != 14)
    {
        machine.clock();
    }

    QVERIFY(machine.viciiBadLine());

    //
    // The normal KERNAL text screen starts at $0400.
    //
    QCOMPARE(
        machine.viciiVideoMatrixBaseAddress(),
        quint16(0x0400));

    //
    // At the first text row of a new frame VCBASE must
    // point to the first character of the video matrix.
    //
    QCOMPARE(
        machine.viciiVideoCounterBase(),
        quint16(0x0000));

    //
    // Cycle 14 copies VCBASE into VC.
    //
    QCOMPARE(
        machine.viciiVideoCounter(),
        quint16(0x0000));

    //
    // Cycle 15 performs the first c-access.
    //
    machine.clock();

    QCOMPARE(
        machine.viciiRasterLine(),
        RasterLine);

    QCOMPARE(
        machine.viciiRasterCycle(),
        quint8(15));

    //
    // The VIC-II must be the only bus master during
    // the c-access.
    //
    QCOMPARE(
        machine.busAccessCount(),
        quint8(1));

    QVERIFY(
        machine.busLastAccessWasRead());

    QVERIFY(
        machine.busLastAccessWasVICII());

    //
    // The first c-access must read the first character
    // of the screen matrix at $0400.
    //
    QCOMPARE(
        machine.busLastAccessAddress(),
        quint16(0x0400));

    QCOMPARE(
        machine.busLastAccessValue(),
        machine.readRAM(0x0400));

    //
    // The fetched character and color must have been
    // stored in the first entries of the VIC-II line buffers.
    //
    QCOMPARE(
        machine.viciiVideoMatrixLine(0),
        machine.readRAM(0x0400));

    QCOMPARE(
        machine.viciiColorLine(0),
        machine.readColorRAM(0x0000));
}




void C64MachineTest::testPerformance()
{
    C64ROMSet romSet(
        QStringLiteral(":/ROMs/OpenROMs/basic.rom"),
        QStringLiteral(":/ROMs/OpenROMs/kernal.rom"),
        QStringLiteral(":/ROMs/OpenROMs/chargen.rom"));

    constexpr quint64 CycleCount = 20000000;
    constexpr quint64 RunCount = 5;
    constexpr quint64 TotalCycles =
        CycleCount * RunCount;

    qint64 totalElapsedNs = 0;

    qint64 minimumElapsedNs =
        std::numeric_limits<qint64>::max();

    qint64 maximumElapsedNs = 0;

    //
    // Execute several independent runs.
    //
    for (quint64 run = 0; run < RunCount; ++run)
    {
        C64Machine machine;

        QVERIFY(machine.loadROMSet(romSet));

        machine.powerOn();

        //
        // Complete the reset sequence and warm up the
        // emulator before starting the measurement.
        //
        machine.runCycles(1000);

        //
        // Measure only complete C64 machine cycles.
        //
        QElapsedTimer timer;
        timer.start();

        for (quint64 cycle = 0;
             cycle < CycleCount;
             ++cycle)
        {
            machine.clock();
        }

        const qint64 elapsedNs =
            timer.nsecsElapsed();

        totalElapsedNs += elapsedNs;

        minimumElapsedNs =
            std::min(
                minimumElapsedNs,
                elapsedNs);

        maximumElapsedNs =
            std::max(
                maximumElapsedNs,
                elapsedNs);
    }

    //
    // Calculate performance information over all runs.
    //
    const double totalElapsedMilliseconds =
        static_cast<double>(totalElapsedNs) /
        1'000'000.0;

    const double totalElapsedSeconds =
        static_cast<double>(totalElapsedNs) /
        1'000'000'000.0;

    const double averageElapsedMilliseconds =
        totalElapsedMilliseconds /
        static_cast<double>(RunCount);

    const double minimumElapsedMilliseconds =
        static_cast<double>(minimumElapsedNs) /
        1'000'000.0;

    const double maximumElapsedMilliseconds =
        static_cast<double>(maximumElapsedNs) /
        1'000'000.0;

    const double megaCyclesPerSecond =
        static_cast<double>(TotalCycles) /
        totalElapsedSeconds /
        1'000'000.0;

    const double palC64MHz =
        static_cast<double>(
            C64::PALTiming.cyclesPerSecond) /
        1'000'000.0;

    const double palRealtime =
        megaCyclesPerSecond /
        palC64MHz;

    qInfo().noquote()
        << QStringLiteral(
               "C64 machine performance\n"
               "  Runs:        %1\n"
               "  Cycles/run:  %2\n"
               "  Total:       %3 cycles\n"
               "  Total time:  %4 ms\n"
               "  Average:     %5 ms/run\n"
               "  Minimum:     %6 ms\n"
               "  Maximum:     %7 ms\n"
               "  Performance: %8 Mcycles/s\n"
               "  PAL C64:     %9 x realtime")
               .arg(RunCount)
               .arg(CycleCount)
               .arg(TotalCycles)
               .arg(
                   totalElapsedMilliseconds,
                   0,
                   'f',
                   3)
               .arg(
                   averageElapsedMilliseconds,
                   0,
                   'f',
                   3)
               .arg(
                   minimumElapsedMilliseconds,
                   0,
                   'f',
                   3)
               .arg(
                   maximumElapsedMilliseconds,
                   0,
                   'f',
                   3)
               .arg(
                   megaCyclesPerSecond,
                   0,
                   'f',
                   3)
               .arg(
                   palRealtime,
                   0,
                   'f',
                   2);

    qInfo().noquote()
        << QStringLiteral(
               "BENCHMARK_RESULT name=C64Machine Mcycles/s=%1")
               .arg(
                   megaCyclesPerSecond,
                   0,
                   'f',
                   3);
}
