#include <QTest>

#include "ROMTest.h"
#include "C64MemoryTest.h"


int main(int argc, char* argv[])
{
    ROMTest romTest;
    C64MemoryTest memoryTest;

    int result = 0;
    result = QTest::qExec(&romTest, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&memoryTest, argc, argv);
    if(result != 0) return result;

    return result;
}

