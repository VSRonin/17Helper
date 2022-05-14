cmake_minimum_required(VERSION 3.14)
find_package(Qt6 6.2 COMPONENTS Widgets Gui Core LinguistTools REQUIRED)
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
    translatablewidgetinterface.h
    translatablewidgetinterface.cpp
    hubwidget.h
    hubwidget.cpp
    loginpage.h
    loginpage.cpp
    loginpage.ui
    initialisationpage.h
    initialisationpage.cpp
    initialisationpage.ui
    downloadmtgatemplatepage.h
    downloadmtgatemplatepage.cpp
    downloadmtgatemplatepage.ui
    downloadoptionspage.h
    downloadoptionspage.cpp
    downloadoptionspage.ui
    logoutpage.cpp
    logoutpage.h
    logoutpage.ui
    downloadprogresspage.h
    downloadprogresspage.cpp
    downloadprogresspage.ui
    uploadoptionspage.h
    uploadoptionspage.cpp
    uploadoptionspage.ui
    customratingspage.h
    customratingspage.cpp
    customratingspage.ui
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
    customratingdelegate.h
    customratingdelegate.cpp
    ratingspinbox.h
    ratingspinbox.cpp
)
set(loggingtoolsUI_SRCS
    forceerrorwidget.h
    forceerrorwidget.cpp
)
source_group(UI FILES ${ui_SRCS})
source_group(Delegates FILES ${delegates_SRCS})
source_group(LoggingTools FILES ${loggingtoolsUI_SRCS})
set(17HelperUi_SRCS
    shuiresources.qrc
    ${ui_SRCS}
    ${loggingtoolsUI_SRCS}
    ${delegates_SRCS}
)
include(GenerateExportHeader)
set_target_properties(17HelperLib PROPERTIES
    AUTOMOC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE
    SOVERSION ${VERSION_MAJOR}
    VERSION ${VERSION_SHORT}
)
add_library(17HelperUiLib SHARED ${17HelperUi_SRCS})
generate_export_header(17HelperUiLib
    EXPORT_MACRO_NAME SHUILIB_EXPORT
    EXPORT_FILE_NAME globaldefs/17helperuilib_global.h
)
qt_add_translations(17HelperUiLib TS_FILES 17HelperUi_en.ts)
add_library(17HelperUiLib::17HelperUiLib ALIAS 17HelperUiLib)
target_include_directories(17HelperUiLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/globaldefs>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(17HelperUiLib PUBLIC
    17HelperLib::17HelperLib
    Qt::Gui
    Qt::Widgets
)
set_target_properties(17HelperUiLib PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE
    SOVERSION ${VERSION_MAJOR}
    VERSION ${VERSION_SHORT}
)
install(TARGETS 17HelperUiLib
    BUNDLE DESTINATION "17Helper"
    RUNTIME DESTINATION "17Helper"
)
include(windeployqt)
WinDeployQt(17HelperUiLib "en" "17Helper")

