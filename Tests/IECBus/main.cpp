#include <QTest>

#include "IECBusTest.h"


int main(int argc, char* argv[])
{
    IECBusTest iecBusTest;

    return QTest::qExec(&iecBusTest, argc, argv);
}

