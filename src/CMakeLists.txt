cmake_minimum_required(VERSION 3.14)
find_package(Qt6 6.2 COMPONENTS Widgets Gui Core LinguistTools REQUIRED)
add_subdirectory(17HelperLib)
add_subdirectory(17HelperUiLib)
add_executable(17Helper guimain.cpp)
target_link_libraries(17Helper PUBLIC
    17HelperUiLib::17HelperUiLib
)
set_target_properties(17Helper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${VERSION_SHORT}
)
install(TARGETS 17Helper
    BUNDLE DESTINATION "17Helper"
    RUNTIME DESTINATION "17Helper"
)
include(windeployqt)
WinDeployQt(17Helper "en" "17Helper")
