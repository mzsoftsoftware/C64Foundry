#include "MOS6510TestFunctional.h"

#include <QTest>
#include <QElapsedTimer>
#include <QFile>


MOS6510TestFunctional::MOS6510TestFunctional()
{
}
MOS6510TestFunctional::~MOS6510TestFunctional()
{
}

void MOS6510TestFunctional::testLoadFunctionalTest()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/FunctionalTest/6502_functional_test.bin"));

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

void MOS6510TestFunctional::testStartFunctionalTest()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/FunctionalTest/6502_functional_test.bin"));

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

void MOS6510TestFunctional::testFunctionalTest()
{
    QFile file(
        QStringLiteral(
            ":/MOS6510/FunctionalTest/6502_functional_test.bin"));

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
