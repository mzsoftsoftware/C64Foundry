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


void C64MachineTest::testPerformance()
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

        QByteArray program;

        //
        // Simple endless loop:
        //
        // $0800: NOP
        // $0801: NOP
        // $0802: NOP
        // $0803: JMP $0800
        //
        program.append(char(0xEA));
        program.append(char(0xEA));
        program.append(char(0xEA));

        program.append(char(0x4C));
        program.append(char(0x00));
        program.append(char(0x08));

        QVERIFY(machine.loadProgram(0x0800, program));

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
