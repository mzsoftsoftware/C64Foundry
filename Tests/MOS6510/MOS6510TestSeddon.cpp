
#include "MOS6510TestSeddon.h"

#include <QFile>
#include <QTest>
#include <QString>

namespace
{
constexpr quint16 LoadAddress          = 0x2000;
constexpr quint16 StartAddress         = 0x2000;
constexpr quint16 TestBeginCallback    = 0x200B;
constexpr quint16 FailCallback         = 0x2013;
constexpr quint16 FinishCallback       = 0x202B;

constexpr quint64 MaxCycles = 5'000'000'000ULL;

constexpr auto ResourcePath =
    ":/MOS6510/SeddonTest/6502-tests-generic.dat";


// Read a zero-terminated test name from emulated RAM.
QString readTestName(
    const C64Memory& memory,
    const quint16 address)
{
    QByteArray name;

    for (quint16 offset = 0; offset < 128; ++offset)
    {
        const quint8 value =
            memory.readRAM(
                static_cast<quint16>(address + offset));

        if (value == 0)
            break;

        name.append(static_cast<char>(value));
    }

    return QString::fromLatin1(name);
}


// The generic suite provides eight bytes for each callback.
// A self-jump lets the host recognize an unambiguous trap.
void installTrap(
    C64Memory& memory,
    const quint16 address)
{
    memory.writeRAM(address, 0x4C); // JMP absolute
    memory.writeRAM(
        static_cast<quint16>(address + 1),
        static_cast<quint8>(address));

    memory.writeRAM(
        static_cast<quint16>(address + 2),
        static_cast<quint8>(address >> 8));
}


// Seddon's state layout:
// A, X, Y, S, P, operand.
QString formatState(
    const C64Memory& memory,
    const quint16 address)
{
    QStringList values;

    for (quint16 offset = 0; offset < 6; ++offset)
    {
        values.append(
            QStringLiteral("%1")
                .arg(
                    memory.readRAM(
                        static_cast<quint16>(
                            address + offset)),
                    2,
                    16,
                    QLatin1Char('0'))
                .toUpper());
    }

    return QStringLiteral(
               "A=%1 X=%2 Y=%3 S=%4 P=%5 O=%6")
        .arg(
            values.at(0),
            values.at(1),
            values.at(2),
            values.at(3),
            values.at(4),
            values.at(5));
}

} // namespace


MOS6510TestSeddon::MOS6510TestSeddon()
{
}

MOS6510TestSeddon::~MOS6510TestSeddon()
{
}


QByteArray MOS6510TestSeddon::loadSeddonBinary()
{
    QFile file(QString::fromLatin1(ResourcePath));

    if (!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}


void MOS6510TestSeddon::testLoadSeddonTest()
{
    const QByteArray data = loadSeddonBinary();

    QVERIFY2(
        !data.isEmpty(),
        "Could not load Seddon generic test binary");

    QVERIFY2(
        data.size() > 0x3B,
        "Seddon binary is unexpectedly short");

    QVERIFY2(
        data.size() <= 0x10000 - LoadAddress,
        "Seddon binary does not fit in RAM");

    setupCpu();

    for (qsizetype offset = 0;
         offset < data.size();
         ++offset)
    {
        m_memory.writeRAM(
            static_cast<quint16>(
                LoadAddress + offset),
            static_cast<quint8>(data.at(offset)));
    }

    for (qsizetype offset = 0;
         offset < data.size();
         ++offset)
    {
        QCOMPARE(
            m_memory.readRAM(
                static_cast<quint16>(
                    LoadAddress + offset)),
            static_cast<quint8>(data.at(offset)));
    }
}


void MOS6510TestSeddon::testStartSeddonTest()
{
    const QByteArray data = loadSeddonBinary();

    QVERIFY(!data.isEmpty());

    setupCpu();

    for (qsizetype offset = 0;
         offset < data.size();
         ++offset)
    {
        m_memory.writeRAM(
            static_cast<quint16>(
                LoadAddress + offset),
            static_cast<quint8>(data.at(offset)));
    }

    m_cpu.setProgramCounter(StartAddress);

    QCOMPARE(
        m_cpu.programCounter(),
        StartAddress);

    // The first CPU clock must fetch the first byte.
    clock();

    verifyRead(
        StartAddress,
        static_cast<quint8>(data.at(0)));

    QCOMPARE(
        m_cpu.programCounter(),
        quint16(StartAddress + 1));
}


void MOS6510TestSeddon::testSeddonTest()
{
    const QByteArray data = loadSeddonBinary();

    QVERIFY2(
        !data.isEmpty(),
        "Could not load Seddon generic test binary");

    QVERIFY(
        data.size() <= 0x10000 - LoadAddress);

    setupCpu();

    for (qsizetype offset = 0;
         offset < data.size();
         ++offset)
    {
        m_memory.writeRAM(
            static_cast<quint16>(
                LoadAddress + offset),
            static_cast<quint8>(data.at(offset)));
    }

    installTrap(m_memory, FailCallback);
    installTrap(m_memory, FinishCallback);

    m_cpu.setProgramCounter(StartAddress);

    QString currentTest;
    quint64 cycles = 0;
    quint64 testsStarted = 0;
    bool insideBeginCallback = false;

    while (cycles < MaxCycles)
    {
        const quint16 pc = m_cpu.programCounter();

        if (pc == FailCallback)
        {
            const quint16 stateAddress =
                static_cast<quint16>(
                    (static_cast<quint16>(
                         m_cpu.yRegister()) << 8) |
                    m_cpu.xRegister());

            const QString input =
                formatState(
                    m_memory,
                    stateAddress);

            const QString actual =
                formatState(
                    m_memory,
                    static_cast<quint16>(
                        stateAddress + 6));

            const QString expected =
                formatState(
                    m_memory,
                    static_cast<quint16>(
                        stateAddress + 12));

            const QString message =
                QStringLiteral(
                    "Seddon FAILED\n"
                    "Test:     %1\n"
                    "Cycles:   %2\n"
                    "Input:    %3\n"
                    "Actual:   %4\n"
                    "Expected: %5")
                    .arg(currentTest)
                    .arg(cycles)
                    .arg(input)
                    .arg(actual)
                    .arg(expected);

            QFAIL(qPrintable(message));
        }

        if (pc == FinishCallback)
        {
            qInfo().noquote()
            << QStringLiteral(
                   "Seddon test suite finished\n"
                   "  Tests started: %1\n"
                   "  Cycles:        %2")
                    .arg(testsStarted)
                    .arg(cycles);

            QVERIFY2(
                testsStarted > 0,
                "Seddon finished without starting tests");

            return;
        }

        // Observe the test-begin callback without modifying
        // its original SEC/RTS implementation.
        if (pc == TestBeginCallback)
        {
            if (!insideBeginCallback)
            {
                const quint16 nameAddress =
                    static_cast<quint16>(
                        (static_cast<quint16>(
                             m_cpu.yRegister()) << 8) |
                        m_cpu.xRegister());

                currentTest =
                    readTestName(
                        m_memory,
                        nameAddress);

                ++testsStarted;

                qInfo().noquote()
                    << QStringLiteral(
                           "Seddon: %1")
                           .arg(currentTest);

                insideBeginCallback = true;
            }
        }
        else
        {
            insideBeginCallback = false;
        }

        clock();
        ++cycles;
    }

    const QString message =
        QStringLiteral(
            "Seddon timeout\n"
            "Last test: %1\n"
            "PC:        $%2\n"
            "Cycles:    %3")
            .arg(currentTest)
            .arg(
                m_cpu.programCounter(),
                4,
                16,
                QLatin1Char('0'))
            .arg(cycles);

    QFAIL(qPrintable(message));
}
