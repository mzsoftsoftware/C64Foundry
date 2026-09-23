#pragma once

#include <QObject>
#include "MOS6510TestBase.h"


class MOS6510TestTransfer : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestTransfer();
    virtual ~MOS6510TestTransfer();

private:
    void initializeRegisters();

private slots:
    void testRegisterTransfer_data();
    void testRegisterTransfer();

    void testTransferXToStackPointer_data();
    void testTransferXToStackPointer();
};
