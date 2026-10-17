add_executable(IECBusTest
    main.cpp
    IECBusTest.h IECBusTest.cpp
)

target_link_libraries(IECBusTest
    PRIVATE
        C64FoundryCore
        Qt6::Test
)

add_test(
    NAME IECBusTest
    COMMAND IECBusTest
)

