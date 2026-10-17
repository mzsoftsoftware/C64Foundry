#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestVICE : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestVICE();
    virtual ~MOS6510TestVICE();

private slots:
    void testAdc00FirstFailure();
    void testAdc00InstructionSequence();
    void testAdc00SelfModifyingCode();
};
