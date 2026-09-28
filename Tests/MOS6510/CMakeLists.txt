
add_executable(MOS6510Test
    main.cpp
    MOS6510TestBase.h MOS6510TestBase.cpp
    MOS6510TestLoad.h MOS6510TestLoad.cpp
    MOS6510TestSpecial.h MOS6510TestSpecial.cpp
    MOS6510TestStore.h MOS6510TestStore.cpp
    MOS6510TestTransfer.h MOS6510TestTransfer.cpp
    MOS6510TestStack.h MOS6510TestStack.cpp
    MOS6510TestLogical.h MOS6510TestLogical.cpp
    MOS6510TestArithmetic.h MOS6510TestArithmetic.cpp
    MOS6510TestDecimal.h MOS6510TestDecimal.cpp
    MOS6510TestIncrementDecrement.h MOS6510TestIncrementDecrement.cpp
    MOS6510TestBusCycles.h MOS6510TestBusCycles.cpp
    MOS6510TestMemoryIncrementDecrement.h MOS6510TestMemoryIncrementDecrement.cpp
    MOS6510TestShiftRotate.h MOS6510TestShiftRotate.cpp
    MOS6510TestBranch.h MOS6510TestBranch.cpp
    MOS6510TestControlFlow.h MOS6510TestControlFlow.cpp
    MOS6510TestCpuControl.h MOS6510TestCpuControl.cpp

    MOS6510TestDormann.h MOS6510TestDormann.cpp
    DormannTest/MOS6510DormannTest.qrc

    MOS6510TestSeddon.h MOS6510TestSeddon.cpp
    SeddonTest/MOS6510SeddonTest.qrc
    MOS6510TestUndocumentedNOP.h MOS6510TestUndocumentedNOP.cpp
    MOS6510TestUndocumentedSBC.h MOS6510TestUndocumentedSBC.cpp
    MOS6510TestUndocumentedARR.h MOS6510TestUndocumentedARR.cpp
    MOS6510TestUndocumentedASR.h MOS6510TestUndocumentedASR.cpp
    MOS6510TestUndocumentedANC.h MOS6510TestUndocumentedANC.cpp
    MOS6510TestUndocumentedDCP.h MOS6510TestUndocumentedDCP.cpp
    MOS6510TestUndocumentedISB.h MOS6510TestUndocumentedISB.cpp
    MOS6510TestUndocumentedLAX.h MOS6510TestUndocumentedLAX.cpp
    MOS6510TestUndocumentedLAS.h MOS6510TestUndocumentedLAS.cpp
    MOS6510TestUndocumentedRLA.h MOS6510TestUndocumentedRLA.cpp
)

set_target_properties(MOS6510Test PROPERTIES
    AUTORCC ON
)

target_link_libraries(MOS6510Test
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

