cmake_minimum_required(VERSION 3.9)
find_package(Qt6 COMPONENTS Widgets Gui Core Network LinguistTools REQUIRED)
set(ui_SRCS 
    mainwindow.cpp
    mainwindow.h
)
set(17_SRCS
    seventeencard.cpp
    seventeencard.h
)
source_group(UI FILES ${ui_SRCS})
source_group(17Lands FILES ${17_SRCS})
set(17Helper_SRCS
    ${ui_SRCS}
    ${17_SRCS}
)
qt6_create_translation(17Helper_QM_FILES ${17Helper_SRCS} 17Helper_en.ts)
add_library(17HelperLib STATIC ${17Helper_SRCS} ${17Helper_QM_FILES})
add_library(17HelperLib::17HelperLib ALIAS 17HelperLib)
target_include_directories(17HelperLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(17HelperLib PUBLIC 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
set_target_properties(17HelperLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
add_executable(17Helper main.cpp)
target_link_libraries(17Helper PUBLIC 17HelperLib::17HelperLib)
set_target_properties(17Helper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
include(InstallRequiredSystemLibraries)
install(
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION runtime
)
install(FILES ${17Helper_QM_FILES} DESTINATION translations)
