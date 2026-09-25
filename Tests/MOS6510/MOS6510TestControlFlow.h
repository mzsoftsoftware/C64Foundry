#pragma once

#include <QObject>

#include "MOS6510TestBase.h"


class MOS6510TestControlFlow : public QObject, public MOS6510TestBase
{
    Q_OBJECT

public:
    explicit MOS6510TestControlFlow();
    virtual ~MOS6510TestControlFlow();

private slots:
    // JMP
    void testJmpAbsolute();
    void testJmpAbsoluteProgramCounterWrapAround();

    void testJmpIndirect();
    void testJmpIndirectPageBoundaryBug();
    void testJmpIndirectPointerWrapAround();

    // JSR
    void testJsr();
    void testJsrStackPointerWrapAround();
    void testJsrProgramCounterWrapAround();

    // RTS
    void testRts();
    void testRtsStackPointerWrapAround();
    void testRtsProgramCounterWrapAround();

    // BRK
    void testBrk();
    void testBrkStackPointerWrapAround();
    void testBrkProgramCounterWrapAround();

    // RTI
    void testRti();
    void testRtiStackPointerWrapAround();
};
