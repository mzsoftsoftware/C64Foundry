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
)

target_link_libraries(MOS6510Test
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

