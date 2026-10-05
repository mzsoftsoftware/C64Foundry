#include <QTest>

#include "MOS6526TestPort.h"
#include "MOS6526TestTimer.h"


int main(int argc, char* argv[])
{
    MOS6526TestPort testPort;
    MOS6526TestTimer testTimer;

    int result = 0;
    result = QTest::qExec(&testPort, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testTimer, argc, argv);
    if(result != 0) return result;

    return result;
}

