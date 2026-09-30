#include <QTest>

#include "C64MachineTest.h"
#include "C64ROMSetTest.h"


int main(int argc, char* argv[])
{
    C64MachineTest machineTest;
    C64ROMSetTest romsetTest;

    int result = 0;
    result = QTest::qExec(&machineTest, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&romsetTest, argc, argv);
    if(result != 0) return result;

    return result;
}

