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

    void testCIARegisterAccess();

    void testPerformance();

};
