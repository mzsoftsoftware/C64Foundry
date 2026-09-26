#pragma once

#include <QObject>
#include <QByteArray>

#include "MOS6510TestBase.h"


class MOS6510TestSeddon : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestSeddon();
    virtual ~MOS6510TestSeddon();

private slots:
    void testLoadSeddonTest();
    void testStartSeddonTest();
    void testSeddonTest();

private:
    QByteArray loadSeddonBinary();

private:
    static constexpr quint16 LoadAddress   = 0x2000;
    static constexpr quint16 StartAddress  = 0x2000;
    static constexpr quint16 FinishCallback = 0x202B;
};
