#include <QTest>

#include "C64MachineTest.h"
#include "C64ROMSetTest.h"
#include "C64InputTest.h"
#include "C64Performance.h"


int main(int argc, char* argv[])
{
    C64ROMSetTest romsetTest;
    C64MachineTest machineTest;
    C64InputTest inputTest;
    C64Performance performance;

    int result = 0;
    result = QTest::qExec(&romsetTest, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&machineTest, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&inputTest, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&performance, argc, argv);
    if(result != 0) return result;

    return result;
}

