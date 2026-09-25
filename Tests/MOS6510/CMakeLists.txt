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
)

target_link_libraries(MOS6510Test
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

