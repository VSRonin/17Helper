cmake_minimum_required(VERSION 3.14)
find_package(Qt6 6.2 COMPONENTS Widgets Gui Core Network Sql LinguistTools Svg REQUIRED)
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
    faileduploadsdialog.cpp
    faileduploadsdialog.h
    faileduploadsdialog.ui
)
qt_add_resources(ui_SRCS rosources.qrc)
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
    slmetricsfiltermodel.h
    slmetricsfiltermodel.cpp
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
set(loggingtools_SRCS
    loggingtools/forceerrorwidget.h
    loggingtools/forceerrorwidget.cpp
    loggingtools/forceerrornetworkmanager.h
    loggingtools/forceerrornetworkmanager.cpp
)
source_group(UI FILES ${ui_SRCS})
source_group(Backend FILES ${backend_SRCS})
source_group(Models FILES ${models_SRCS})
source_group(Delegates FILES ${delegates_SRCS})
source_group(LoggingTools FILES ${loggingtools_SRCS})
set(17Helper_SRCS
    ${backend_SRCS}
    ${models_SRCS}
    ${delegates_SRCS}
)
set(17HelperUi_SRCS
    ${ui_SRCS}
    ${loggingtools_SRCS}
)
add_library(17HelperLib STATIC ${17Helper_SRCS} ${17Helper_QM_FILES})
qt_add_translations(17HelperLib TS_FILES 17Helper_en.ts 17Helper_it.ts QM_FILES_OUTPUT_VARIABLE 17Helper_QM_FILES)
add_library(17HelperLib::17HelperLib ALIAS 17HelperLib)
target_compile_definitions(17HelperLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(17HelperLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(17HelperLib PUBLIC 
    Qt::Core
    Qt::Network
    Qt::Sql
    QtModelUtilities::QtModelUtilities
)
set_target_properties(17HelperLib PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
add_library(17HelperUiLib STATIC ${17HelperUi_SRCS} ${17HelperUi_QM_FILES})
qt_add_translations(17HelperUiLib TS_FILES 17HelperUi_en.ts 17HelperUi_it.ts QM_FILES_OUTPUT_VARIABLE 17HelperUi_QM_FILES)
add_library(17HelperUiLib::17HelperUiLib ALIAS 17HelperUiLib)
target_compile_definitions(17HelperUiLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(17HelperUiLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(17HelperUiLib PUBLIC
    17HelperLib::17HelperLib
    Qt::Gui
    Qt::Widgets
    Qt::Svg
)
set_target_properties(17HelperUiLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
add_executable(17Helper guimain.cpp)
target_link_libraries(17Helper PUBLIC
    17HelperLib::17HelperLib
    17HelperUiLib::17HelperUiLib
)
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
install(FILES ${17Helper_QM_FILES} ${17HelperUi_QM_FILES} DESTINATION "17Helper/translations")
install(DIRECTORY "${OPENSSL_ROOT_DIR}/bin/"
    RUNTIME DESTINATION "17Helper"
    FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}"
)
include(windeployqt)
WinDeployQt(17Helper "en,it" "17Helper")
