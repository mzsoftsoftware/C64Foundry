#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestBusCycles : public QObject,
                             public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestBusCycles();
    virtual ~MOS6510TestBusCycles();

private:
    void clock();
    void verifyRead(quint16 address, quint8 value);
    void verifyWrite(quint16 address, quint8 value);
    void verifyNoAccess();

private slots:
    void init();

    // ADC
    void testAdcImmediate();
    void testAdcZeroPage();
    void testAdcZeroPageX();
    void testAdcZeroPageXWrapAround();
    void testAdcAbsolute();
    void testAdcAbsoluteXWithoutPageCrossing();
    void testAdcAbsoluteXWithPageCrossing();
    void testAdcAbsoluteYWithoutPageCrossing();
    void testAdcAbsoluteYWithPageCrossing();
    void testAdcIndexedIndirect();
    void testAdcIndexedIndirectWrapAround();
    void testAdcIndirectIndexedWithoutPageCrossing();
    void testAdcIndirectIndexedWithPageCrossing();
    void testAdcIndirectIndexedWrapAround();

    // AND
    void testAndImmediate();
    void testAndZeroPage();
    void testAndZeroPageX();
    void testAndZeroPageXWrapAround();
    void testAndAbsolute();
    void testAndAbsoluteXWithoutPageCrossing();
    void testAndAbsoluteXWithPageCrossing();
    void testAndAbsoluteYWithoutPageCrossing();
    void testAndAbsoluteYWithPageCrossing();
    void testAndIndexedIndirect();
    void testAndIndexedIndirectWrapAround();
    void testAndIndirectIndexedWithoutPageCrossing();
    void testAndIndirectIndexedWithPageCrossing();
    void testAndIndirectIndexedWrapAround();

    // ASL
    void testAslAccumulator();
    void testAslZeroPage();
    void testAslZeroPageX();
    void testAslZeroPageXWrapAround();
    void testAslAbsolute();
    void testAslAbsoluteXWithoutPageCrossing();
    void testAslAbsoluteXWithPageCrossing();

    // Branches
    void testBccNotTaken();
    void testBccTakenWithoutPageCrossing();
    void testBccTakenWithPageCrossing();

    void testBcsNotTaken();
    void testBcsTakenWithoutPageCrossing();
    void testBcsTakenWithPageCrossing();

    void testBeqNotTaken();
    void testBeqTakenWithoutPageCrossing();
    void testBeqTakenWithPageCrossing();

    void testBmiNotTaken();
    void testBmiTakenWithoutPageCrossing();
    void testBmiTakenWithPageCrossing();

    void testBneNotTaken();
    void testBneTakenWithoutPageCrossing();
    void testBneTakenWithPageCrossing();

    void testBplNotTaken();
    void testBplTakenWithoutPageCrossing();
    void testBplTakenWithPageCrossing();

    void testBvcNotTaken();
    void testBvcTakenWithoutPageCrossing();
    void testBvcTakenWithPageCrossing();

    void testBvsNotTaken();
    void testBvsTakenWithoutPageCrossing();
    void testBvsTakenWithPageCrossing();

    // BIT
    void testBitZeroPage();
    void testBitAbsolute();

    // BRK
    void testBrk();
    void testBrkStackPointerWrapAround();

    // Flag instructions
    void testClc();
    void testCld();
    void testCli();
    void testClv();
    void testSec();
    void testSed();
    void testSei();

    // CMP
    void testCmpImmediate();
    void testCmpZeroPage();
    void testCmpZeroPageX();
    void testCmpZeroPageXWrapAround();
    void testCmpAbsolute();
    void testCmpAbsoluteXWithoutPageCrossing();
    void testCmpAbsoluteXWithPageCrossing();
    void testCmpAbsoluteYWithoutPageCrossing();
    void testCmpAbsoluteYWithPageCrossing();
    void testCmpIndexedIndirect();
    void testCmpIndexedIndirectWrapAround();
    void testCmpIndirectIndexedWithoutPageCrossing();
    void testCmpIndirectIndexedWithPageCrossing();
    void testCmpIndirectIndexedWrapAround();

    // CPX
    void testCpxImmediate();
    void testCpxZeroPage();
    void testCpxAbsolute();

    // CPY
    void testCpyImmediate();
    void testCpyZeroPage();
    void testCpyAbsolute();

    // DEC
    void testDecZeroPage();
    void testDecZeroPageX();
    void testDecZeroPageXWrapAround();
    void testDecAbsolute();
    void testDecAbsoluteXWithoutPageCrossing();
    void testDecAbsoluteXWithPageCrossing();

    // Register decrement
    void testDex();
    void testDey();

    // EOR
    void testEorImmediate();
    void testEorZeroPage();
    void testEorZeroPageX();
    void testEorZeroPageXWrapAround();
    void testEorAbsolute();
    void testEorAbsoluteXWithoutPageCrossing();
    void testEorAbsoluteXWithPageCrossing();
    void testEorAbsoluteYWithoutPageCrossing();
    void testEorAbsoluteYWithPageCrossing();
    void testEorIndexedIndirect();
    void testEorIndexedIndirectWrapAround();
    void testEorIndirectIndexedWithoutPageCrossing();
    void testEorIndirectIndexedWithPageCrossing();
    void testEorIndirectIndexedWrapAround();

    // INC
    void testIncZeroPage();
    void testIncZeroPageX();
    void testIncZeroPageXWrapAround();
    void testIncAbsolute();
    void testIncAbsoluteXWithoutPageCrossing();
    void testIncAbsoluteXWithPageCrossing();

    // Register increment
    void testInx();
    void testIny();

    // JMP
    void testJmpAbsolute();
    void testJmpIndirect();
    void testJmpIndirectPageBoundaryBug();

    // JSR
    void testJsr();
    void testJsrStackPointerWrapAround();

    // LDA
    void testLdaImmediate();
    void testLdaZeroPage();
    void testLdaZeroPageX();
    void testLdaZeroPageXWrapAround();
    void testLdaAbsolute();
    void testLdaAbsoluteXWithoutPageCrossing();
    void testLdaAbsoluteXWithPageCrossing();
    void testLdaAbsoluteYWithoutPageCrossing();
    void testLdaAbsoluteYWithPageCrossing();
    void testLdaIndexedIndirect();
    void testLdaIndexedIndirectWrapAround();
    void testLdaIndirectIndexedWithoutPageCrossing();
    void testLdaIndirectIndexedWithPageCrossing();
    void testLdaIndirectIndexedWrapAround();

    // LDX
    void testLdxImmediate();
    void testLdxZeroPage();
    void testLdxZeroPageY();
    void testLdxZeroPageYWrapAround();
    void testLdxAbsolute();
    void testLdxAbsoluteYWithoutPageCrossing();
    void testLdxAbsoluteYWithPageCrossing();

    // LDY
    void testLdyImmediate();
    void testLdyZeroPage();
    void testLdyZeroPageX();
    void testLdyZeroPageXWrapAround();
    void testLdyAbsolute();
    void testLdyAbsoluteXWithoutPageCrossing();
    void testLdyAbsoluteXWithPageCrossing();

    // LSR
    void testLsrAccumulator();
    void testLsrZeroPage();
    void testLsrZeroPageX();
    void testLsrZeroPageXWrapAround();
    void testLsrAbsolute();
    void testLsrAbsoluteXWithoutPageCrossing();
    void testLsrAbsoluteXWithPageCrossing();

    // NOP
    void testNop();

    // ORA
    void testOraImmediate();
    void testOraZeroPage();
    void testOraZeroPageX();
    void testOraZeroPageXWrapAround();
    void testOraAbsolute();
    void testOraAbsoluteXWithoutPageCrossing();
    void testOraAbsoluteXWithPageCrossing();
    void testOraAbsoluteYWithoutPageCrossing();
    void testOraAbsoluteYWithPageCrossing();
    void testOraIndexedIndirect();
    void testOraIndexedIndirectWrapAround();
    void testOraIndirectIndexedWithoutPageCrossing();
    void testOraIndirectIndexedWithPageCrossing();
    void testOraIndirectIndexedWrapAround();

    // Stack
    void testPha();
    void testPhaStackPointerWrapAround();

    void testPhp();
    void testPhpStackPointerWrapAround();

    void testPla();
    void testPlaStackPointerWrapAround();

    void testPlp();
    void testPlpStackPointerWrapAround();

    // ROL
    void testRolAccumulator();
    void testRolZeroPage();
    void testRolZeroPageX();
    void testRolZeroPageXWrapAround();
    void testRolAbsolute();
    void testRolAbsoluteXWithoutPageCrossing();
    void testRolAbsoluteXWithPageCrossing();

    // ROR
    void testRorAccumulator();
    void testRorZeroPage();
    void testRorZeroPageX();
    void testRorZeroPageXWrapAround();
    void testRorAbsolute();
    void testRorAbsoluteXWithoutPageCrossing();
    void testRorAbsoluteXWithPageCrossing();

    // RTI
    void testRti();
    void testRtiStackPointerWrapAround();

    // RTS
    void testRts();
    void testRtsStackPointerWrapAround();

    // SBC
    void testSbcImmediate();
    void testSbcZeroPage();
    void testSbcZeroPageX();
    void testSbcZeroPageXWrapAround();
    void testSbcAbsolute();
    void testSbcAbsoluteXWithoutPageCrossing();
    void testSbcAbsoluteXWithPageCrossing();
    void testSbcAbsoluteYWithoutPageCrossing();
    void testSbcAbsoluteYWithPageCrossing();
    void testSbcIndexedIndirect();
    void testSbcIndexedIndirectWrapAround();
    void testSbcIndirectIndexedWithoutPageCrossing();
    void testSbcIndirectIndexedWithPageCrossing();
    void testSbcIndirectIndexedWrapAround();

    // STA
    void testStaZeroPage();
    void testStaZeroPageX();
    void testStaZeroPageXWrapAround();
    void testStaAbsolute();
    void testStaAbsoluteXWithoutPageCrossing();
    void testStaAbsoluteXWithPageCrossing();
    void testStaAbsoluteYWithoutPageCrossing();
    void testStaAbsoluteYWithPageCrossing();
    void testStaIndexedIndirect();
    void testStaIndexedIndirectWrapAround();
    void testStaIndirectIndexedWithoutPageCrossing();
    void testStaIndirectIndexedWithPageCrossing();
    void testStaIndirectIndexedWrapAround();

    // STX
    void testStxZeroPage();
    void testStxZeroPageY();
    void testStxZeroPageYWrapAround();
    void testStxAbsolute();

    // STY
    void testStyZeroPage();
    void testStyZeroPageX();
    void testStyZeroPageXWrapAround();
    void testStyAbsolute();

    // Transfers
    void testTax();
    void testTay();
    void testTsx();
    void testTxa();
    void testTxs();
    void testTya();
};