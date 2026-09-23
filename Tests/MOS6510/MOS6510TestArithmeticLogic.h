#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestArithmeticLogic : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestArithmeticLogic();
    virtual ~MOS6510TestArithmeticLogic();

private:

private slots:


};
