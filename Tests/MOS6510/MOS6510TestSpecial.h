#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestSpecial : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestSpecial();
    virtual ~MOS6510TestSpecial();

private:


private slots:
    void testNOP();

    void testFlagInstructions_data();
    void testFlagInstructions();
};
