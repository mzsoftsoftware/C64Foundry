
add_executable(MOS6526Test
    main.cpp
    MOS6526TestPort.h MOS6526TestPort.cpp
)

set_target_properties(MOS6510Test PROPERTIES
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
