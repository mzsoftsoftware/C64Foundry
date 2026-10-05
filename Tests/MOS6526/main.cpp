#include <QTest>

#include "MOS6526TestPort.h"


int main(int argc, char* argv[])
{
    MOS6526TestPort testPort;

    int result = 0;
    result = QTest::qExec(&testPort, argc, argv);
    if(result != 0) return result;

    return result;
}

