#include <QTest>

#include "ROMTest.h"


int main(int argc, char* argv[])
{
    ROMTest romTest;

    int result = 0;
    result = QTest::qExec(&romTest, argc, argv);
    if(result != 0) return result;

    return result;
}

