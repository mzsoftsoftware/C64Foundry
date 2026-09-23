#include <QTest>

#include "MOS6510TestLoad.h"
#include "MOS6510TestSpecial.h"


int main(int argc, char* argv[])
{
    MOS6510TestLoad testLoad;
    MOS6510TestSpecial testSpecial;

    int result = 0;
    result = QTest::qExec(&testLoad, argc, argv);
    result = QTest::qExec(&testSpecial, argc, argv);

    return result;
}
