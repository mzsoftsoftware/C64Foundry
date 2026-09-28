#include <QTest>

#include "MOS6510TestLoad.h"
#include "MOS6510TestStore.h"
#include "MOS6510TestTransfer.h"
#include "MOS6510TestStack.h"
#include "MOS6510TestLogical.h"
#include "MOS6510TestArithmetic.h"
#include "MOS6510TestDecimal.h"
#include "MOS6510TestIncrementDecrement.h"
#include "MOS6510TestMemoryIncrementDecrement.h"
#include "MOS6510TestShiftRotate.h"
#include "MOS6510TestBranch.h"
#include "MOS6510TestSpecial.h"
#include "MOS6510TestControlFlow.h"
#include "MOS6510TestBusCycles.h"
#include "MOS6510TestCpuControl.h"

#include "MOS6510TestUndocumentedNOP.h"
#include "MOS6510TestUndocumentedSBC.h"
#include "MOS6510TestUndocumentedARR.h"
#include "MOS6510TestUndocumentedASR.h"
#include "MOS6510TestUndocumentedANC.h"
#include "MOS6510TestUndocumentedDCP.h"
#include "MOS6510TestUndocumentedISB.h"
#include "MOS6510TestUndocumentedLAX.h"
#include "MOS6510TestUndocumentedLAS.h"

#include "MOS6510TestDormann.h"
#include "MOS6510TestSeddon.h"


int main(int argc, char* argv[])
{
    MOS6510TestLoad testLoad;
    MOS6510TestStore testStore;
    MOS6510TestTransfer testTransfer;
    MOS6510TestStack testStack;
    MOS6510TestLogical testLogical;
    MOS6510TestArithmetic testArithmetic;
    MOS6510TestDecimal testDecimal;
    MOS6510TestIncrementDecrement testIncDec;
    MOS6510TestMemoryIncrementDecrement testMemoryIncrementDecrement;
    MOS6510TestShiftRotate testShiftRotate;
    MOS6510TestBranch testBranch;
    MOS6510TestSpecial testSpecial;
    MOS6510TestControlFlow testControlFlow;
    MOS6510TestBusCycles testBusCycles;
    MOS6510TestCpuControl testCpuControl;

    MOS6510TestUndocumentedNOP testUndocNOP;
    MOS6510TestUndocumentedSBC testUndocSBC;
    MOS6510TestUndocumentedARR testUndocARR;
    MOS6510TestUndocumentedASR testUndocASR;
    MOS6510TestUndocumentedANC testUndocANC;
    MOS6510TestUndocumentedDCP testUndocDCP;
    MOS6510TestUndocumentedISB testUndocISB;
    MOS6510TestUndocumentedLAX testUndocLAX;
    MOS6510TestUndocumentedLAS testUndocLAS;

    MOS6510TestDormann testDormann;
    MOS6510TestSeddon testSeddon;

    int result = 0;
    result = QTest::qExec(&testLoad, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testStore, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testTransfer, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testStack, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testLogical, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testArithmetic, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testDecimal, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testIncDec, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testMemoryIncrementDecrement, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testShiftRotate, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testBranch, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testSpecial, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testControlFlow, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testBusCycles, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testCpuControl, argc, argv);
    if(result != 0) return result;

    // --------------------------

    result = QTest::qExec(&testUndocNOP, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocSBC, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocARR, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocASR, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocANC, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocDCP, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocISB, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocLAX, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&testUndocLAS, argc, argv);
    if(result != 0) return result;

    // --------------------------

    /*result = QTest::qExec(&testDormann, argc, argv);
    if(result != 0) return result;*/

    result = QTest::qExec(&testSeddon, argc, argv);
    if(result != 0) return result;


    return result;
}

