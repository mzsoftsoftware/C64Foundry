cmake_minimum_required(VERSION 3.19)

project(C64Foundry
    VERSION 0.1.0
    LANGUAGES CXX
)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    LinguistTools
    Test
)

qt_standard_project_setup()

# -----------------------------------------------------------------------------
# C64Foundry Core
# -----------------------------------------------------------------------------
add_library(C64FoundryCore STATIC
    C64/C64Machine.h C64/C64Machine.cpp
    C64/C64Timing.h
    C64/C64ROMSet.h C64/C64ROMSet.cpp

    C64/CPU/MOS6510.h C64/CPU/MOS6510.cpp
    C64/CPU/MOS6510Instruction.h
    C64/CPU/MOS6510InstructionTable.h C64/CPU/MOS6510InstructionTable.cpp
    C64/CPU/MOS6510MicroOperation.h
    C64/CPU/MOS6510StatusRegister.h

    C64/CIA/MOS6526Port.h
    C64/CIA/MOS6526.h C64/CIA/MOS6526.cpp

    C64/Bus/C64Bus.h C64/Bus/C64Bus.cpp

    C64/Memory/C64Memory.h C64/Memory/C64Memory.cpp
    C64/Memory/RAM.h C64/Memory/RAM.cpp
    C64/Memory/ROM.h C64/Memory/ROM.cpp
    C64/Memory/ColorRAM.h C64/Memory/ColorRAM.cpp

    C64/VIC-II/VIC-II.h C64/VIC-II/VIC-II.cpp

    C64/Input/C64Keyboard.h C64/Input/C64Keyboard.cpp
    C64/Input/C64CIA1Port.h C64/Input/C64CIA1Port.cpp
)

target_include_directories(C64FoundryCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(C64FoundryCore
    PUBLIC
        Qt6::Core
)

# -----------------------------------------------------------------------------
# C64Foundry Application
# -----------------------------------------------------------------------------
qt_add_executable(C64Foundry
    WIN32 MACOSX_BUNDLE

    App/main.cpp

    App/MainWindow.cpp
    App/MainWindow.h
    App/MainWindow.ui

    Configuration/ConfigurationManager.h Configuration/ConfigurationManager.cpp
    Configuration/ConfigurationStorage.h Configuration/ConfigurationStorage.cpp
    Configuration/C64Configuration.h
    Configuration/HostPlatform.h Configuration/HostPlatform.cpp
    Configuration/ROMSetDetector.h Configuration/ROMSetDetector.cpp
    Configuration/C64KeyboardConfiguration.h Configuration/C64KeyboardConfiguration.cpp
    Configuration/C64KeyboardDefaults.h Configuration/C64KeyboardDefaults.cpp

    Gui/Emulator/EmulatorControlWidget.h Gui/Emulator/EmulatorControlWidget.cpp Gui/Emulator/EmulatorControlWidget.ui
    Gui/Video/VideoWindowWidget.h Gui/Video/VideoWindowWidget.cpp

    Emulator/EmulatorSpeed.h
    Emulator/EmulatorController.h Emulator/EmulatorController.cpp
    Emulator/EmulatorWorker.h Emulator/EmulatorWorker.cpp
    Emulator/EmulatorThread.h Emulator/EmulatorThread.cpp

    Output/Video/VideoController.h Output/Video/VideoController.cpp
    Output/Video/VideoWorker.h Output/Video/VideoWorker.cpp
    Output/Video/VideoThread.h Output/Video/VideoThread.cpp

    Input/InputEvent.h
    Input/Keyboard/KeyboardController.h Input/Keyboard/KeyboardController.cpp

    Resources/C64Foundry.qrc


)

target_include_directories(C64Foundry
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(C64Foundry PROPERTIES
    AUTORCC ON
)

target_link_libraries(C64Foundry
    PRIVATE
        C64FoundryCore
        Qt6::Core
        Qt6::Widgets
)

qt_add_translations(
    TARGETS C64Foundry
    TS_FILES C64Foundry_de_DE.ts
)

target_link_libraries(C64Foundry
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(Tests EXCLUDE_FROM_ALL)
endif()
