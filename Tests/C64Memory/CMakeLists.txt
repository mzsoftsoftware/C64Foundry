
add_executable(C64MemoryTest
    main.cpp
    ROMTest.h ROMTest.cpp
    C64MemoryTest.h C64MemoryTest.cpp
)

set_target_properties(C64MemoryTest PROPERTIES
    AUTORCC ON
)

target_link_libraries(C64MemoryTest
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

add_test(
    NAME C64MemoryTest
    COMMAND C64MemoryTest
)

