#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestStore : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestStore();
    virtual ~MOS6510TestStore();

private:
    void initializeRegisters();
    void verifyRegisters(quint8 accumulator, quint8 xRegister, quint8 yRegister, quint8 status);

private slots:
    void testZeroPageStore_data();
    void testZeroPageStore();

    void testAbsoluteStore_data();
    void testAbsoluteStore();

    void testIndexedStore_data();
    void testIndexedStore();

    void testIndexedIndirectStore_data();
    void testIndexedIndirectStore();

    void testIndirectIndexedStore_data();
    void testIndirectIndexedStore();
};
