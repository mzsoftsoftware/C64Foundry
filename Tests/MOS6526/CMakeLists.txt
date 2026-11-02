
add_executable(MOS6526Test
    main.cpp
    MOS6526TestPort.h MOS6526TestPort.cpp
    MOS6526TestTimer.h MOS6526TestTimer.cpp
    MOS6526TestTOD.h MOS6526TestTOD.cpp
    MOS6526TestRegister.h MOS6526TestRegister.cpp
)

set_target_properties(MOS6526Test PROPERTIES
    AUTORCC ON
)

target_link_libraries(MOS6526Test
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

add_test(
    NAME MOS6526Test
    COMMAND MOS6526Test
)
