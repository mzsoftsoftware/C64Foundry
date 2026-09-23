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

private slots:


};
