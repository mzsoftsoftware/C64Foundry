#pragma once

#include <QObject>


class C64BusTest : public QObject
{
    Q_OBJECT

public:
    explicit C64BusTest();
    virtual ~C64BusTest();

private slots:
    void testInitialCpuPortLines();
    void testSetCpuPortLines();

    void testBasicROMMapping();
    void testKernalROMMapping();
    void testCharacterROMAndIOMapping();

    void testReadMemoryMapping();
    void testWriteRAMBelowROM();

    void testColorRAMMapping();

    void testVICIIRegisterMapping();
    void testVICMemoryRead();
    void testVICMemoryAddressMask();
    void testVICCharacterROM();
    void testVICCharacterROMBoundaries();

    void testAEC();

    void testVICReadAccess();
    void testVICCharacterROMReadAccess();
};
