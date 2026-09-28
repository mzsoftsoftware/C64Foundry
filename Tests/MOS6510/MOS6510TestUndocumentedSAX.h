#pragma once

#include "MOS6510TestBase.h"

#include <QObject>


class MOS6510TestUndocumentedSAX : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestUndocumentedSAX();
    virtual ~MOS6510TestUndocumentedSAX();

private slots:
    void testZeroPage_data();
    void testZeroPage();

    void testZeroPageY_data();
    void testZeroPageY();
    void testZeroPageYWrap();

    void testAbsolute_data();
    void testAbsolute();

    void testIndexedIndirect_data();
    void testIndexedIndirect();
    void testIndexedIndirectPointerWrap();

    void testExhaustive();
};
