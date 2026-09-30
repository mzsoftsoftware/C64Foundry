#include <QTest>

#include "C64BusTest.h"


int main(int argc, char* argv[])
{
    C64BusTest c64Bustest;

    int result = 0;
    result = QTest::qExec(&c64Bustest, argc, argv);
    if(result != 0) return result;

    return result;
}

