#pragma once

#include <QObject>
#include "MOS6510TestBase.h"

enum class LoadRegister
{
    Accumulator,
    X,
    Y
};
enum class IndexRegister
{
    X,
    Y
};


class MOS6510TestLoad : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestLoad();
    virtual ~MOS6510TestLoad();

private:
    void initializeRegisters();
    void verifyLoadedRegister(LoadRegister registerType, quint8 value);
    quint8 expectedLoadStatus(quint8 status, quint8 value) const;

private slots:
    void testImmediateLoad_data();
    void testImmediateLoad();
    void testImmediateLoadPcWrap_data();
    void testImmediateLoadPcWrap();

    void testZeroPageLoad_data();
    void testZeroPageLoad();
    void testZeroPageLoadPcWrap();

    void testZeroPageIndexedLoad_data();
    void testZeroPageIndexedLoad();

};
