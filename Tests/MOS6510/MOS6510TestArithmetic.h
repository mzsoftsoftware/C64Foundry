#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestArithmetic : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestArithmetic();
    virtual ~MOS6510TestArithmetic();

private:
    void initializeRegisters();
    quint8 expectedAdcStatus(quint8 status, quint8 accumulator, quint8 operand) const;

private slots:
    void testAdcImmediate_data();
    void testAdcImmediate();

};
