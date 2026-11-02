#include <QTest>

#include "MOS6526TestPort.h"
#include "MOS6526TestTimer.h"
#include "MOS6526TestTOD.h"
#include "MOS6526TestRegister.h"


int main(int argc, char* argv[])
{
    MOS6526TestPort testPort;
    MOS6526TestTimer testTimer;
    MOS6526TestTOD testTod;
    MOS6526TestRegister testRegister;

    int result = 0;
    result = QTest::qExec(&testPort, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testTimer, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testTod, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testRegister, argc, argv);
    if(result != 0) return result;

    return result;
}

