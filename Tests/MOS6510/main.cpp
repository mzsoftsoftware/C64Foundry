#include <QTest>

#include "MOS6510TestLoad.h"
#include "MOS6510TestStore.h"
#include "MOS6510TestTransfer.h"
#include "MOS6510TestStack.h"
#include "MOS6510TestLogical.h"
#include "MOS6510TestArithmetic.h"
#include "MOS6510TestDecimal.h"
#include "MOS6510TestSpecial.h"


int main(int argc, char* argv[])
{
    MOS6510TestLoad testLoad;
    MOS6510TestStore testStore;
    MOS6510TestTransfer testTransfer;
    MOS6510TestStack testStack;
    MOS6510TestLogical testLogical;
    MOS6510TestArithmetic testArithmetic;
    MOS6510TestDecimal testDecimal;
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

    result = QTest::qExec(&testStack, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testLogical, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testArithmetic, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testDecimal, argc, argv);
    if(result != 0)
        return result;

    result = QTest::qExec(&testSpecial, argc, argv);
    if(result != 0)
        return result;

    return result;
}

