cmake_minimum_required(VERSION 3.19)
project(C64Foundry LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(C64Foundry
    WIN32 MACOSX_BUNDLE
    App/main.cpp
    App/MainWindow.cpp
    App/MainWindow.h
    App/MainWindow.ui
    C64/C64Machine.h C64/C64Machine.cpp
    C64/CPU/MOS6510.h C64/CPU/MOS6510.cpp
    C64/Bus/C64Bus.h C64/Bus/C64Bus.cpp
    C64/Memory/C64Memory.h C64/Memory/C64Memory.cpp
    C64/Memory/RAM.h C64/Memory/RAM.cpp
    C64/Memory/BasicROM.h C64/Memory/BasicROM.cpp
    C64/Memory/KernalROM.h C64/Memory/KernalROM.cpp
    C64/Memory/ColorRAM.h C64/Memory/ColorRAM.cpp
    C64/Memory/CharROM.h C64/Memory/CharROM.cpp
)

qt_add_translations(
    TARGETS C64Foundry
    TS_FILES C64Foundry_de_DE.ts
)

target_link_libraries(C64Foundry
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS C64Foundry
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET C64Foundry
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
