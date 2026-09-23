#include <QTest>

#include "MOS6510TestLoad.h"
#include "MOS6510TestStore.h"
#include "MOS6510TestTransfer.h"
#include "MOS6510TestSpecial.h"


int main(int argc, char* argv[])
{
    MOS6510TestLoad testLoad;
    MOS6510TestStore testStore;
    MOS6510TestTransfer testTransfer;
    MOS6510TestSpecial testSpecial;

    int result = 0;
    result = QTest::qExec(&testLoad, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testStore, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testTransfer, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testSpecial, argc, argv);
    if(result != 0)
        return result;

    return result;
}
