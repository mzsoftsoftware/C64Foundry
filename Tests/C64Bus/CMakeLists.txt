
add_executable(C64BusTest
    main.cpp
    C64BusTest.h C64BusTest.cpp
)

set_target_properties(C64BusTest PROPERTIES
    AUTORCC ON
)

target_link_libraries(C64BusTest
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

add_test(
    NAME C64BusTest
    COMMAND C64BusTest
)
