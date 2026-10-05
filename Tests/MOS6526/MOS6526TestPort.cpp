#include "MOS6526TestPort.h"

#include <QtTest>

#include "C64/CIA/MOS6526.h"


MOS6526TestPort::MOS6526TestPort()
{
}

MOS6526TestPort::~MOS6526TestPort()
{
}


void MOS6526TestPort::testPortARegisterWrite()
{
    MOS6526 cia;

    cia.writeRegister(0x00, 0x5A);

    QCOMPARE(cia.readRegister(0x00), quint8(0x5A));
}
