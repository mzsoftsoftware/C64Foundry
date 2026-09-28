#include "MOS6510TestBlargg.h"

#include <QFile>
#include <QTest>


MOS6510TestBlargg::MOS6510TestBlargg()
{
}

MOS6510TestBlargg::~MOS6510TestBlargg()
{
}


void MOS6510TestBlargg::addBlarggTestData()
{
    QTest::addColumn<QString>("romName");

    QTest::newRow("01-basics")
        << QStringLiteral("01-basics.nes");

    QTest::newRow("02-implied")
        << QStringLiteral("02-implied.nes");

    QTest::newRow("03-immediate")
        << QStringLiteral("03-immediate.nes");

    QTest::newRow("04-zero_page")
        << QStringLiteral("04-zero_page.nes");

    QTest::newRow("05-zp_xy")
        << QStringLiteral("05-zp_xy.nes");

    QTest::newRow("06-absolute")
        << QStringLiteral("06-absolute.nes");

    QTest::newRow("07-abs_xy")
        << QStringLiteral("07-abs_xy.nes");

    QTest::newRow("08-ind_x")
        << QStringLiteral("08-ind_x.nes");

    QTest::newRow("09-ind_y")
        << QStringLiteral("09-ind_y.nes");

    QTest::newRow("10-branches")
        << QStringLiteral("10-branches.nes");

    QTest::newRow("11-stack")
        << QStringLiteral("11-stack.nes");

    QTest::newRow("12-jmp_jsr")
        << QStringLiteral("12-jmp_jsr.nes");

    QTest::newRow("13-rts")
        << QStringLiteral("13-rts.nes");

    QTest::newRow("14-rti")
        << QStringLiteral("14-rti.nes");

    QTest::newRow("15-brk")
        << QStringLiteral("15-brk.nes");

    QTest::newRow("16-special")
        << QStringLiteral("16-special.nes");
}


void MOS6510TestBlargg::clearTestMemory()
{
    for (quint32 address = 0;
         address < 0x10000;
         ++address)
    {
        m_memory.writeRAM(
            static_cast<quint16>(address),
            0x00);
    }
}


QByteArray MOS6510TestBlargg::loadBlarggRom(
    const QString& romName)
{
    const QString resourceName =
        QStringLiteral(
            ":/MOS6510/BlarggTest/%1")
            .arg(romName);

    QFile file(resourceName);

    if (!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}


QByteArray MOS6510TestBlargg::extractPrgRom(
    const QByteArray& rom)
{
    //
    // iNES header:
    //
    //   0-3   "NES" $1A
    //   4     number of 16 KiB PRG banks
    //
    if (rom.size() < 16)
        return {};

    if (static_cast<quint8>(rom.at(0)) != 0x4E ||
        static_cast<quint8>(rom.at(1)) != 0x45 ||
        static_cast<quint8>(rom.at(2)) != 0x53 ||
        static_cast<quint8>(rom.at(3)) != 0x1A)
    {
        return {};
    }

    const quint8 prgBanks =
        static_cast<quint8>(
            rom.at(4));

    //
    // The individual Blargg instruction test ROMs
    // contain two 16 KiB PRG banks = 32 KiB.
    //
    if (prgBanks != 2)
        return {};

    //
    // Bit 2 of flags 6 indicates a 512-byte trainer.
    //
    const quint8 flags6 =
        static_cast<quint8>(
            rom.at(6));

    const qsizetype trainerSize =
        (flags6 & 0x04) != 0
            ? 512
            : 0;

    const qsizetype prgOffset =
        16 + trainerSize;

    constexpr qsizetype PrgSize =
        32 * 1024;

    if (rom.size() < prgOffset + PrgSize)
        return {};

    return rom.mid(
        prgOffset,
        PrgSize);
}


QString MOS6510TestBlargg::readBlarggOutput()
{
    QByteArray output;

    //
    // Keep diagnostic output bounded.
    //
    constexpr quint16 MaximumLength =
        1024;

    for (quint16 offset = 0;
         offset < MaximumLength;
         ++offset)
    {
        const quint8 value =
            m_memory.readRAM(
                static_cast<quint16>(
                    OutputAddress + offset));

        if (value == 0)
            break;

        output.append(
            static_cast<char>(value));
    }

    return QString::fromLatin1(output);
}


void MOS6510TestBlargg::testLoadBlarggTest_data()
{
    addBlarggTestData();
}


void MOS6510TestBlargg::testLoadBlarggTest()
{
    QFETCH(
        QString,
        romName);

    const QByteArray rom =
        loadBlarggRom(
            romName);

    const QByteArray errorMessage =
        QStringLiteral(
            "Could not load Blargg %1")
            .arg(romName)
            .toLatin1();

    QVERIFY2(
        !rom.isEmpty(),
        errorMessage.constData());

    QVERIFY2(
        rom.size() >= 16,
        "Blargg ROM is unexpectedly short");

    //
    // Verify iNES magic:
    //
    //   4E 45 53 1A
    //
    QCOMPARE(
        static_cast<quint8>(
            rom.at(0)),
        quint8(0x4E));

    QCOMPARE(
        static_cast<quint8>(
            rom.at(1)),
        quint8(0x45));

    QCOMPARE(
        static_cast<quint8>(
            rom.at(2)),
        quint8(0x53));

    QCOMPARE(
        static_cast<quint8>(
            rom.at(3)),
        quint8(0x1A));

    //
    // Every individual instr_test-v5 ROM used here
    // must contain exactly 32 KiB PRG.
    //
    QCOMPARE(
        static_cast<quint8>(
            rom.at(4)),
        quint8(2));

    const QByteArray prg =
        extractPrgRom(
            rom);

    QCOMPARE(
        prg.size(),
        qsizetype(32 * 1024));

    setupCpu();
    clearTestMemory();

    //
    // Map PRG to:
    //
    //   $8000-$FFFF
    //
    for (qsizetype offset = 0;
         offset < prg.size();
         ++offset)
    {
        m_memory.writeRAM(
            static_cast<quint16>(
                PrgLoadAddress + offset),
            static_cast<quint8>(
                prg.at(offset)));
    }

    //
    // Verify the complete mapped PRG image.
    //
    for (qsizetype offset = 0;
         offset < prg.size();
         ++offset)
    {
        QCOMPARE(
            m_memory.readRAM(
                static_cast<quint16>(
                    PrgLoadAddress + offset)),
            static_cast<quint8>(
                prg.at(offset)));
    }

    //
    // The reset vector must point into PRG ROM.
    //
    const quint16 resetVector =
        static_cast<quint16>(
            m_memory.readRAM(0xFFFC) |
            (static_cast<quint16>(
                 m_memory.readRAM(0xFFFD)) << 8));

    QVERIFY2(
        resetVector >= PrgLoadAddress,
        "Blargg reset vector does not point into PRG ROM");
}


void MOS6510TestBlargg::testBlarggTest()
{
    runBlarggTest(
        QStringLiteral("01-basics.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("02-implied.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("03-immediate.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("04-zero_page.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("05-zp_xy.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("06-absolute.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("07-abs_xy.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("08-ind_x.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("09-ind_y.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("10-branches.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("11-stack.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("12-jmp_jsr.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("13-rts.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("14-rti.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("15-brk.nes"));

    if (QTest::currentTestFailed())
        return;

    runBlarggTest(
        QStringLiteral("16-special.nes"));
}


void MOS6510TestBlargg::runBlarggTest(
    const QString& romName)
{
    const QByteArray rom =
        loadBlarggRom(
            romName);

    const QByteArray loadErrorMessage =
        QStringLiteral(
            "Could not load Blargg %1")
            .arg(romName)
            .toLatin1();

    QVERIFY2(
        !rom.isEmpty(),
        loadErrorMessage.constData());

    const QByteArray prg =
        extractPrgRom(
            rom);

    const QByteArray prgErrorMessage =
        QStringLiteral(
            "Could not extract PRG ROM from Blargg %1")
            .arg(romName)
            .toLatin1();

    QVERIFY2(
        prg.size() == 32 * 1024,
        prgErrorMessage.constData());

    setupCpu();
    clearTestMemory();

    //
    // Map PRG image to $8000-$FFFF.
    //
    for (qsizetype offset = 0;
         offset < prg.size();
         ++offset)
    {
        m_memory.writeRAM(
            static_cast<quint16>(
                PrgLoadAddress + offset),
            static_cast<quint8>(
                prg.at(offset)));
    }

    //
    // Read reset vector from the ROM.
    //
    const quint16 resetVector =
        static_cast<quint16>(
            m_memory.readRAM(0xFFFC) |
            (static_cast<quint16>(
                 m_memory.readRAM(0xFFFD)) << 8));

    QVERIFY2(
        resetVector >= PrgLoadAddress,
        "Invalid Blargg reset vector");

    //
    // CPU reset sequencing itself is already tested
    // independently. Start directly at the ROM's
    // reset entry point.
    //
    m_cpu.setProgramCounter(
        resetVector);

    //
    // Deliberately generous safety limit.
    //
    // Unsupported opcodes are detected immediately
    // through MOS6510::stopped(), so this normally
    // cannot waste time on a stopped CPU.
    //
    constexpr quint64 MaxCycles =
        1'000'000'000ULL;

    quint64 cycles = 0;
    bool interfaceSeen = false;

    while (cycles < MaxCycles)
    {
        clock();
        ++cycles;

        //
        // An unimplemented opcode places the MOS6510
        // into Stopped immediately after its opcode
        // fetch cycle.
        //
        if (m_cpu.stopped())
        {
            const quint16 opcodeAddress =
                static_cast<quint16>(
                    m_cpu.programCounter() - 1);

            const quint8 opcode =
                m_memory.readRAM(
                    opcodeAddress);

            const QString output =
                readBlarggOutput();

            const QString message =
                QStringLiteral(
                    "Blargg %1 CPU stopped\n"
                    "Opcode:  $%2\n"
                    "Address: $%3\n"
                    "PC:      $%4\n"
                    "Cycles:  %5\n"
                    "Output:\n%6")
                    .arg(romName)
                    .arg(
                        opcode,
                        2,
                        16,
                        QLatin1Char('0'))
                    .arg(
                        opcodeAddress,
                        4,
                        16,
                        QLatin1Char('0'))
                    .arg(
                        m_cpu.programCounter(),
                        4,
                        16,
                        QLatin1Char('0'))
                    .arg(cycles)
                    .arg(output)
                    .toUpper();

            QFAIL(
                qPrintable(message));
        }

        const quint8 status =
            m_memory.readRAM(
                StatusAddress);

        //
        // Ignore $6000 until Blargg has initialized
        // its memory-based result interface.
        //
        if (!interfaceSeen)
        {
            if (status == 0x80)
                interfaceSeen = true;

            continue;
        }

        //
        // $80 = test still running.
        //
        if (status == 0x80)
            continue;

        const QString output =
            readBlarggOutput();

        //
        // $81 = host reset requested.
        //
        if (status == 0x81)
        {
            const QString message =
                QStringLiteral(
                    "Blargg %1 requested RESET\n"
                    "PC:     $%2\n"
                    "Cycles: %3\n"
                    "Output:\n%4")
                    .arg(romName)
                    .arg(
                        m_cpu.programCounter(),
                        4,
                        16,
                        QLatin1Char('0'))
                    .arg(cycles)
                    .arg(output);

            QFAIL(
                qPrintable(message));
        }

        //
        // $00 = PASS.
        //
        if (status == 0x00)
            return;

        //
        // Some Blargg tests have known differences because
        // instr_test-v5 targets the NES Ricoh CPU rather than
        // the NMOS MOS6510.
        //
        // Only explicitly verified differences are accepted.
        //
        if (isExpectedFailure(
                romName,
                status,
                output))
        {
            return;
        }

        //
        // $01-$7F = FAIL.
        //
        const QString message =
            QStringLiteral(
                "Blargg %1 failed\n"
                "Result: $%2\n"
                "PC:     $%3\n"
                "Cycles: %4\n"
                "Output:\n%5")
                .arg(romName)
                .arg(
                    status,
                    2,
                    16,
                    QLatin1Char('0'))
                .arg(
                    m_cpu.programCounter(),
                    4,
                    16,
                    QLatin1Char('0'))
                .arg(cycles)
                .arg(output);

        QFAIL(
            qPrintable(message));
    }

    //
    // Safety timeout.
    //
    const QString output =
        readBlarggOutput();

    const QString message =
        QStringLiteral(
            "Blargg %1 timeout\n"
            "PC:     $%2\n"
            "Cycles: %3\n"
            "Output:\n%4")
            .arg(romName)
            .arg(
                m_cpu.programCounter(),
                4,
                16,
                QLatin1Char('0'))
            .arg(cycles)
            .arg(output);

    QFAIL(
        qPrintable(message));
}

bool MOS6510TestBlargg::isExpectedFailure(
    const QString& romName,
    const quint8 status,
    const QString& output) const
{
    //
    // Blargg instr_test-v5 targets the NES Ricoh CPU.
    //
    // Its decimal-mode behaviour differs from the NMOS
    // MOS6510 used by the C64. In addition, unstable
    // undocumented opcodes may have CPU-specific results.
    //
    // Accept only explicitly known and verified
    // differences. Any additional or changed output must
    // remain a test failure.
    //
    if (status != 0x01)
        return false;

    if (romName == QStringLiteral("03-immediate.nes"))
    {
        const QString ExpectedOutput =
            QStringLiteral(
                "69 ADC #n\n"
                "E9 SBC #n\n"
                "EB SBC #n\n"
                "6B ARR #n\n"
                "AB ATX #n\n"
                "\n"
                "03-immediate\n"
                "\n"
                "Failed\n");

        return output == ExpectedOutput;
    }
    if (romName == QStringLiteral("04-zero_page.nes"))
    {
        const QString ExpectedOutput =
            QStringLiteral(
                "65 ADC z\n"
                "E5 SBC z\n"
                "67 RRA z\n"
                "E7 ISC z\n"
                "\n"
                "04-zero_page\n"
                "\n"
                "Failed\n");

        return output == ExpectedOutput;
    }
    if (romName == QStringLiteral("05-zp_xy.nes"))
    {
        const QString ExpectedOutput =
            QStringLiteral(
                "75 ADC z,X\n"
                "F5 SBC z,X\n"
                "77 RRA z,X\n"
                "F7 ISC z,X\n"
                "\n"
                "05-zp_xy\n"
                "\n"
                "Failed\n");

        return output == ExpectedOutput;
    }
    if (romName == QStringLiteral("06-absolute.nes"))
    {
        const QString ExpectedOutput =
            QStringLiteral(
                "6D ADC a\n"
                "ED SBC a\n"
                "6F RRA abs\n"
                "EF ISC abs\n"
                "\n"
                "06-absolute\n"
                "\n"
                "Failed\n");

        return output == ExpectedOutput;
    }


    return false;
}
