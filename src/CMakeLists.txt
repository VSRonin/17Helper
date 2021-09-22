cmake_minimum_required(VERSION 3.9)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Gui Core Network REQUIRED)
set(ui_SRCS 
    mainwindow.cpp
    mainwindow.h
)
source_group(UI FILES ${ui_SRCS})
add_library(17HelperLib STATIC ${ui_SRCS})
add_library(17HelperLib::17HelperLib ALIAS 17HelperLib)
target_include_directories(17HelperLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(17HelperLib PUBLIC 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)
set_target_properties(17HelperLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
add_executable(17Helper main.cpp)
target_link_libraries(17Helper PUBLIC 17HelperLib::17HelperLib)
set_target_properties(17Helper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
