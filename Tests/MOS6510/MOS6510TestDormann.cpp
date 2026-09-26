#include "MOS6510TestDormann.h"

#include <QTest>
#include <QElapsedTimer>
#include <QFile>
#include <algorithm>
#include <limits>


MOS6510TestDormann::MOS6510TestDormann()
{
}
MOS6510TestDormann::~MOS6510TestDormann()
{
}

void MOS6510TestDormann::testLoadDormannTest()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/DormannTest/6502_functional_test.bin"));

    QVERIFY2(
        file.open(QIODevice::ReadOnly),
        "Could not open 6502_functional_test.bin");

    const QByteArray data = file.readAll();

    QCOMPARE(data.size(), 65536);

    //
    // Verify a few known bytes from the listing.
    //
    // $0400: CLD
    //
    QCOMPARE(
        static_cast<quint8>(data.at(0x0400)),
        quint8(0xD8));

    //
    // $3469: JMP $3469
    //
    QCOMPARE(
        static_cast<quint8>(data.at(0x3469)),
        quint8(0x4C));

    QCOMPARE(
        static_cast<quint8>(data.at(0x346A)),
        quint8(0x69));

    QCOMPARE(
        static_cast<quint8>(data.at(0x346B)),
        quint8(0x34));
}

void MOS6510TestDormann::testStartDormannTest()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/DormannTest/6502_functional_test.bin"));

    QVERIFY2(
        file.open(QIODevice::ReadOnly),
        "Could not open 6502_functional_test.bin");

    const QByteArray data = file.readAll();

    QCOMPARE(data.size(), 65536);

    setupCpu();

    //
    // Load complete 64 KiB memory image.
    //
    for (qsizetype address = 0; address < data.size(); ++address)
    {
        m_memory.writeRAM(
            static_cast<quint16>(address),
            static_cast<quint8>(data.at(address)));
    }

    //
    // Klaus Dormann functional test starts at $0400.
    //
    m_cpu.setProgramCounter(0x0400);

    QCOMPARE(m_memory.readRAM(0x0400), quint8(0xD8));
    QCOMPARE(m_cpu.programCounter(), quint16(0x0400));

    //
    // $0400: CLD
    //
    // C1: opcode fetch
    //
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x0401));

    //
    // C2: implied dummy read + execute CLD
    //
    clock();

    QCOMPARE(m_cpu.programCounter(), quint16(0x0401));
    QCOMPARE(
        m_cpu.statusFlag(MOS6510StatusFlag::Decimal),
        false);
}

void MOS6510TestDormann::testDormannTest()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/DormannTest/6502_functional_test.bin"));

    QVERIFY2(
        file.open(QIODevice::ReadOnly),
        "Could not open 6502_functional_test.bin");

    const QByteArray data = file.readAll();

    QCOMPARE(data.size(), 65536);

    setupCpu();

    //
    // Load complete 64 KiB memory image.
    //
    // Loading the test image is intentionally outside the
    // performance measurement.
    //
    for (qsizetype address = 0; address < data.size(); ++address)
    {
        m_memory.writeRAM(
            static_cast<quint16>(address),
            static_cast<quint8>(data.at(address)));
    }

    constexpr quint16 StartAddress   = 0x0400;
    constexpr quint16 SuccessAddress = 0x3469;

    constexpr quint64 MaxCycles =
        100'000'000;

    constexpr quint64 ExpectedCycles =
        96'241'367;

    constexpr double PalC64MHz =
        0.985248;

    m_cpu.setProgramCounter(StartAddress);

    quint64 cycles = 0;
    bool successOpcodeFetched = false;

    //
    // Start performance measurement only after the complete
    // memory image has been loaded and the CPU is ready to run.
    //
    QElapsedTimer timer;
    timer.start();

    //
    // Run until the opcode of the success trap is actually
    // fetched from $3469.
    //
    while (cycles < MaxCycles)
    {
        clock();
        ++cycles;

        if (m_bus.lastAccessType() == C64Bus::AccessType::Read &&
            m_bus.lastAccessAddress() == SuccessAddress &&
            m_bus.lastAccessValue() == 0x4C)
        {
            successOpcodeFetched = true;
            break;
        }
    }

    QVERIFY2(
        successOpcodeFetched,
        qPrintable(
            QStringLiteral(
                "6502 functional test did not reach success trap. "
                "PC=$%1, cycles=%2")
                .arg(
                    m_cpu.programCounter(),
                    4,
                    16,
                    QLatin1Char('0'))
                .arg(cycles)));

    //
    // Success trap:
    //
    // $3469: JMP $3469
    //
    // C1 has already been executed by the loop above:
    //
    //   R $3469 = $4C
    //
    // After the opcode fetch PC points to the low byte
    // of the target address.
    //
    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x346A));

    //
    // C2:
    //
    // Read target address low byte.
    //
    clock();
    ++cycles;

    verifyRead(
        0x346A,
        0x69);

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(0x346B));

    //
    // C3:
    //
    // Read target address high byte and perform the jump.
    //
    clock();
    ++cycles;

    verifyRead(
        0x346B,
        0x34);

    QCOMPARE(
        m_cpu.programCounter(),
        SuccessAddress);

    //
    // Stop performance measurement immediately after the
    // complete success JMP has been executed.
    //
    const qint64 elapsedNs =
        timer.nsecsElapsed();

    //
    // The cycle count is deterministic for this exact
    // functional-test image and CPU start state.
    //
    QCOMPARE(
        cycles,
        ExpectedCycles);

    //
    // Calculate performance information.
    //
    const double elapsedMilliseconds =
        static_cast<double>(elapsedNs) /
        1'000'000.0;

    const double elapsedSeconds =
        static_cast<double>(elapsedNs) /
        1'000'000'000.0;

    const double megaCyclesPerSecond =
        static_cast<double>(cycles) /
        elapsedSeconds /
        1'000'000.0;

    const double palRealtime =
        megaCyclesPerSecond /
        PalC64MHz;

    qInfo().noquote()
        << QStringLiteral(
               "6502 functional test passed\n"
               "  Cycles:      %1\n"
               "  Time:        %2 ms\n"
               "  Performance: %3 Mcycles/s\n"
               "  PAL C64:     %4 x realtime")
               .arg(cycles)
               .arg(
                   elapsedMilliseconds,
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
}

void MOS6510TestDormann::testDormannTestPerformance()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/DormannTest/6502_functional_test.bin"));

    QVERIFY2(
        file.open(QIODevice::ReadOnly),
        "Could not open 6502_functional_test.bin");

    const QByteArray data = file.readAll();

    QCOMPARE(data.size(), 65536);

    constexpr quint16 StartAddress   = 0x0400;
    constexpr quint16 SuccessAddress = 0x3469;

    constexpr quint64 ExpectedCycles =
        96'241'367;

    constexpr quint64 RunCount =
        10;

    constexpr quint64 TotalCycles =
        ExpectedCycles * RunCount;

    constexpr double PalC64MHz =
        0.985248;

    qint64 totalElapsedNs = 0;
    qint64 minimumElapsedNs =
        std::numeric_limits<qint64>::max();
    qint64 maximumElapsedNs = 0;

    //
    // Execute several independent runs.
    //
    // CPU initialization and loading the memory image are
    // intentionally outside the performance measurement.
    //
    for (quint64 run = 0; run < RunCount; ++run)
    {
        setupCpu();

        //
        // Restore the complete 64 KiB memory image before
        // every run because the functional test modifies RAM.
        //
        for (qsizetype address = 0;
             address < data.size();
             ++address)
        {
            m_memory.writeRAM(
                static_cast<quint16>(address),
                static_cast<quint8>(data.at(address)));
        }

        m_cpu.setProgramCounter(StartAddress);

        //
        // Measure only the actual emulator clock loop.
        //
        QElapsedTimer timer;
        timer.start();

        for (quint64 cycle = 0;
             cycle < ExpectedCycles;
             ++cycle)
        {
            clock();
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

        //
        // After exactly ExpectedCycles clocks the complete
        //
        //   $3469: JMP $3469
        //
        // must have been executed.
        //
        QCOMPARE(
            m_cpu.programCounter(),
            SuccessAddress);

        //
        // Verify that the success trap is still present.
        //
        QCOMPARE(
            m_memory.readRAM(0x3469),
            quint8(0x4C));

        QCOMPARE(
            m_memory.readRAM(0x346A),
            quint8(0x69));

        QCOMPARE(
            m_memory.readRAM(0x346B),
            quint8(0x34));
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

    const double palRealtime =
        megaCyclesPerSecond /
        PalC64MHz;

    qInfo().noquote()
        << QStringLiteral(
               "6502 functional test performance\n"
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
               .arg(ExpectedCycles)
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
}
