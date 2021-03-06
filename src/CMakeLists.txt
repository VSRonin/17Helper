cmake_minimum_required(VERSION 3.14)
find_package(Qt6 COMPONENTS Widgets Gui Core Network LinguistTools REQUIRED)
find_package(OpenSSL REQUIRED)
set(ui_SRCS 
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)
set(backend_SRCS
    seventeencard.cpp
    seventeencard.h
    mtgahcard.h
    mtgahcard.cpp
    worker.h
    worker.cpp
)
set(models_SRCS
    ratingsmodel.h
    ratingsmodel.cpp
)
set(delegates_SRCS
    ratingsdelegate.h
    ratingsdelegate.cpp
)
source_group(UI FILES ${ui_SRCS})
source_group(Backend FILES ${backend_SRCS})
source_group(Models FILES ${models_SRCS})
source_group(Delegates FILES ${delegates_SRCS})
set(17Helper_SRCS
    ${ui_SRCS}
    ${backend_SRCS}
    ${models_SRCS}
    ${delegates_SRCS}
)
qt6_create_translation(17Helper_QM_FILES ${17Helper_SRCS} 17Helper_en.ts)
add_library(17HelperLib STATIC ${17Helper_SRCS} ${17Helper_QM_FILES})
add_library(17HelperLib::17HelperLib ALIAS 17HelperLib)
target_compile_definitions(17HelperLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
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
install(TARGETS 17Helper
    BUNDLE DESTINATION "17Helper"
    RUNTIME DESTINATION "17Helper"
)
install(FILES ${17Helper_QM_FILES} DESTINATION "17Helper/translations")
install(DIRECTORY "${OPENSSL_ROOT_DIR}/bin/"
    RUNTIME DESTINATION "17Helper"
    FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}"
)
include(InstallRequiredSystemLibraries)
include(windeployqt)
WinDeployQt(17Helper en "17Helper")
include(InstallRequiredSystemLibraries)
install(
    PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION "17Helper"
)
