#include "C64Performance.h"

#include <QTest>
#include <QTemporaryFile>
#include <QByteArray>
#include <QElapsedTimer>

#include <algorithm>
#include <limits>

#include "C64/C64Machine.h"
#include "C64/C64ROMSet.h"

void C64Performance::testPerformance()
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
