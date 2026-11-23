cmake_minimum_required(VERSION 3.21)
project(MeridianBankGUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

qt_add_executable(MeridianBankGUI
    main.cpp
    BankTypes.h
    BankService.h BankService.cpp
    UiHelpers.h
    LoginWindow.h LoginWindow.cpp
    Pages.h Pages.cpp
    MainWindow.h MainWindow.cpp
)

target_link_libraries(MeridianBankGUI PRIVATE Qt6::Widgets)

if(WIN32)
    set_target_properties(MeridianBankGUI PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Help IDEs launch the app with the supplied text data files in the working directory.
set_property(TARGET MeridianBankGUI PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if(MSVC)
    target_compile_options(MeridianBankGUI PRIVATE /W4)
else()
    target_compile_options(MeridianBankGUI PRIVATE -Wall -Wextra -Wpedantic)
endif()
