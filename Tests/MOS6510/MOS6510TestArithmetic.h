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

    void testAdcZeroPage_data();
    void testAdcZeroPage();

    void testAdcZeroPageX_data();
    void testAdcZeroPageX();

    void testAdcAbsolute_data();
    void testAdcAbsolute();

    void testAdcAbsoluteX_data();
    void testAdcAbsoluteX();

    void testAdcAbsoluteY_data();
    void testAdcAbsoluteY();

    void testAdcIndexedIndirect_data();
    void testAdcIndexedIndirect();

    void testAdcIndirectIndexed_data();
    void testAdcIndirectIndexed();


    void testSbcImmediate_data();
    void testSbcImmediate();

    void testSbcZeroPage_data();
    void testSbcZeroPage();

    void testSbcZeroPageX_data();
    void testSbcZeroPageX();

    void testSbcAbsolute_data();
    void testSbcAbsolute();

    void testSbcAbsoluteX_data();
    void testSbcAbsoluteX();

    void testSbcAbsoluteY_data();
    void testSbcAbsoluteY();

    void testSbcIndexedIndirect_data();
    void testSbcIndexedIndirect();

    void testSbcIndirectIndexed_data();
    void testSbcIndirectIndexed();


    void testCmpImmediate_data();
    void testCmpImmediate();

    void testCmpZeroPage_data();
    void testCmpZeroPage();

    void testCmpZeroPageX_data();
    void testCmpZeroPageX();

    void testCmpAbsolute_data();
    void testCmpAbsolute();

    void testCmpAbsoluteX_data();
    void testCmpAbsoluteX();

    void testCmpAbsoluteY_data();
    void testCmpAbsoluteY();

    void testCmpIndexedIndirect_data();
    void testCmpIndexedIndirect();

    void testCmpIndirectIndexed_data();
    void testCmpIndirectIndexed();


    void testCpxImmediate_data();
    void testCpxImmediate();

    void testCpxZeroPage_data();
    void testCpxZeroPage();

    void testCpxAbsolute_data();
    void testCpxAbsolute();


    void testCpyImmediate_data();
    void testCpyImmediate();

    void testCpyZeroPage_data();
    void testCpyZeroPage();

    void testCpyAbsolute_data();
    void testCpyAbsolute();
};
