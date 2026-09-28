#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

#include "MOS6510TestBase.h"


class MOS6510TestBlargg : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestBlargg();
    virtual ~MOS6510TestBlargg();

private slots:
    void testLoadBlarggTest_data();
    void testLoadBlarggTest();

    void testBlarggTest();

private:
    QByteArray loadBlarggRom(const QString& romName);
    QByteArray extractPrgRom(const QByteArray& rom);
    QString readBlarggOutput();

    void addBlarggTestData();
    void clearTestMemory();
    void runBlarggTest(const QString& romName);

private:
    static constexpr quint16 PrgLoadAddress = 0x8000;
    static constexpr quint16 StatusAddress = 0x6000;
    static constexpr quint16 OutputAddress = 0x6004;
};
