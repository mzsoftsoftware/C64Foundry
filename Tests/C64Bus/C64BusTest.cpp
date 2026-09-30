#include "C64BusTest.h"

#include <QTest>

#include "C64/Bus/C64Bus.h"


C64BusTest::C64BusTest()
{
}
C64BusTest::~C64BusTest()
{
}

void C64BusTest::testInitialCpuPortLines()
{
    C64Bus bus;
    QCOMPARE(bus.cpuPortLines(), quint8(0x07));
}

void C64BusTest::testSetCpuPortLines()
{
    C64Bus bus;

    bus.setCpuPortLines(0x02);
    QCOMPARE(bus.cpuPortLines(), quint8(0x02));

    bus.setCpuPortLines(0x05);
    QCOMPARE(bus.cpuPortLines(), quint8(0x05));

    bus.setCpuPortLines(0xFF);
    QCOMPARE(bus.cpuPortLines(), quint8(0x07));
}
