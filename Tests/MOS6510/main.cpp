#include <QTest>

#include "MOS6510TestLoad.h"


int main(int argc, char* argv[])
{
    MOS6510TestLoad testLoad;

    const int result = QTest::qExec(&testLoad, argc, argv);

    return result;
}
