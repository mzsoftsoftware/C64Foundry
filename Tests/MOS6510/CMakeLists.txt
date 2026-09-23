add_executable(MOS6510Test
    main.cpp
    MOS6510TestBase.h MOS6510TestBase.cpp
    MOS6510TestLoad.h MOS6510TestLoad.cpp
    MOS6510TestSpecial.h MOS6510TestSpecial.cpp
)

target_link_libraries(MOS6510Test
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

