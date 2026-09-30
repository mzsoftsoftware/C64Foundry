#include <QTest>

#include "VICIITest.h"


int main(int argc, char* argv[])
{
    VICIITest viciitest;

    int result = 0;
    result = QTest::qExec(&viciitest, argc, argv);
    if(result != 0) return result;

    return result;
}

