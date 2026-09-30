
add_executable(C64MachineTest
    main.cpp
    C64MachineTest.h C64MachineTest.cpp
    C64ROMSetTest.h C64ROMSetTest.cpp
)

set_target_properties(C64MachineTest PROPERTIES
    AUTORCC ON
)

target_link_libraries(C64MachineTest
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

add_test(
    NAME C64MachineTest
    COMMAND C64MachineTest
)

