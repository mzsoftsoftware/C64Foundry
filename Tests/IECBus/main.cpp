#include <QTest>

#include "IECBusTest.h"
#include "C64CIA2PortTest.h"

int main(int argc, char* argv[])
{
    IECBusTest iecBusTest;
    C64CIA2PortTest cia2PortTest;

    int result = 0;
    result = QTest::qExec(&iecBusTest, argc, argv);
    if(result != 0) return result;

    result = QTest::qExec(&cia2PortTest, argc, argv);
    if(result != 0) return result;

    return result;
}
