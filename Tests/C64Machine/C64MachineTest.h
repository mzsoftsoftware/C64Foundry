#pragma once

#include <QObject>


class C64MachineTest : public QObject
{
    Q_OBJECT

private slots:
    void testLoadROMSet();
    void testLoadInvalidROMSet();

    void testRunCycles();

    void testLoadProgram();
    void testExecuteProgram();

    void testVICIIRasterIRQ();
    void testVICIIAEC();
    void testVICIIBAWrite();
    void testVICIIAECStopsCPUAccess();
    void testVICIIBadLineCPUStall();

    void testCIARegisterAccess();
    void testCIAClock();
    void testCIA1IRQ();

    void testROMBootScreenRAMStable();
    void testROMBootColorRAMStable();
    void testROMBootCursorBlink();
    void testROMBootVICIICAccess();

    void testPerformance();

};
