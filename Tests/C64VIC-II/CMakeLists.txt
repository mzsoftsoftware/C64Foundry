

add_executable(C64VICIITest
    main.cpp
    VICIITest.h VICIITest.cpp
)

set_target_properties(C64BusTest PROPERTIES
    AUTORCC ON
)

target_link_libraries(C64VICIITest
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

add_test(
    NAME C64VICIITest
    COMMAND C64VICIITest
)
