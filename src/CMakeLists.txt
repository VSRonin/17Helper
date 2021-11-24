cmake_minimum_required(VERSION 3.14)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Gui Core Network Sql LinguistTools REQUIRED)
find_package(OpenSSL REQUIRED)
include(FetchContent)
FetchContent_Declare(QtModelUtilitiesFC
    GIT_REPOSITORY https://github.com/VSRonin/QtModelUtilities.git
    GIT_TAG master
)
FetchContent_MakeAvailable(QtModelUtilitiesFC)
set(ui_SRCS 
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    centralwidget.cpp
    centralwidget.h
    centralwidget.ui
)
set(backend_SRCS
    globals.h
    globals.cpp
    worker.h
    worker.cpp
    mainobject.h
    mainobject.cpp
    configmanager.h
    configmanager.cpp
)
set(models_SRCS
    ratingsmodel.h
    ratingsmodel.cpp
    checkableproxy.h
    checkableproxy.cpp
    slratingsmodel.h
    slratingsmodel.cpp
    offlinesqlquerymodel.h
    offlinesqlquerymodel.cpp
    offlinesqltable.h
    offlinesqltable.cpp
    setsmodel.h
    setsmodel.cpp
    setsfiltermodel.h
    setsfiltermodel.cpp
    customratingmodel.h
    customratingmodel.cpp
)
set(delegates_SRCS
    ratingsdelegate.h
    ratingsdelegate.cpp
    textdatedelegate.h
    textdatedelegate.cpp
    percentagedelegate.h
    percentagedelegate.cpp
    decimaldelegate.h
    decimaldelegate.cpp
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
qt_create_translation(17Helper_QM_FILES ${17Helper_SRCS} 17Helper_en.ts)
add_library(17HelperLib STATIC ${17Helper_SRCS} ${17Helper_QM_FILES})
add_library(17HelperLib::17HelperLib ALIAS 17HelperLib)
target_compile_definitions(17HelperLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(17HelperLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(17HelperLib PUBLIC 
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Network
    Qt::Sql
    QtModelUtilities::QtModelUtilities
)
set_target_properties(17HelperLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
add_executable(17Helper guimain.cpp)
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
