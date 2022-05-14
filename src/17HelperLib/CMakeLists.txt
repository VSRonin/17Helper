cmake_minimum_required(VERSION 3.14)
find_package(Qt6 6.2 COMPONENTS Core Network Sql LinguistTools REQUIRED)
find_package(OpenSSL REQUIRED)
include(FetchContent)
cmake_policy(SET CMP0077 NEW)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)
set(BUILD_MODELSERIALISATION OFF)
set(BUILD_INSERTPROXY OFF)
set(BUILD_ROOTINDEXPROXY OFF)
set(BUILD_GENERICMODEL OFF)
set(MODEL_UTILITIES_INSTALL OFF)
FetchContent_Declare(QtModelUtilitiesFC
    GIT_REPOSITORY https://github.com/VSRonin/QtModelUtilities.git
    GIT_TAG master
)
FetchContent_MakeAvailable(QtModelUtilitiesFC)
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
    nocheckproxy.h
)
set(loggingtools_SRCS
    forceerrornetworkmanager.h
    forceerrornetworkmanager.cpp
)
source_group(Backend FILES ${backend_SRCS})
source_group(Models FILES ${models_SRCS})
source_group(LoggingTools FILES ${loggingtools_SRCS})
set(17Helper_SRCS
    ${backend_SRCS}
    ${loggingtools_SRCS}
    ${models_SRCS}
)
include(GenerateExportHeader)
add_library(17HelperLib SHARED ${17Helper_SRCS})
generate_export_header(17HelperLib
    EXPORT_MACRO_NAME SHLIB_EXPORT
    EXPORT_FILE_NAME globaldefs/17helperlib_global.h
)
qt_add_translations(17HelperLib TS_FILES 17Helper_en.ts)
add_library(17HelperLib::17HelperLib ALIAS 17HelperLib)
target_compile_definitions(17HelperLib PUBLIC QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
target_include_directories(17HelperLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/globaldefs>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(17HelperLib PUBLIC 
    Qt::Core
    Qt::Network
    Qt::Sql
    QtModelUtilities::QtModelUtilities
)
set_target_properties(17HelperLib PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE
    SOVERSION ${VERSION_MAJOR}
    VERSION ${VERSION_SHORT}
)
install(TARGETS 17HelperLib
    BUNDLE DESTINATION "17Helper"
    RUNTIME DESTINATION "17Helper"
)
install(DIRECTORY "${OPENSSL_ROOT_DIR}/bin/"
    RUNTIME DESTINATION "17Helper"
    FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}"
)
include(windeployqt)
WinDeployQt(17HelperLib "en" "17Helper")
