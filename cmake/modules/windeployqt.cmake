macro(WinDeployQt TargetName Translations InstallDestination)
    if(Qt6Core_FOUND AND WIN32)
        get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)
        execute_process(
            COMMAND "${_qt6_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt6_install_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        add_custom_command(TARGET ${TargetName}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
            COMMAND ${CMAKE_COMMAND} -E env "PATH=${qt6_install_prefix}/bin$<SEMICOLON>$ENV{PATH}" "${qt6_install_prefix}/bin/windeployqt.exe" --translations ${Translations} --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:${TargetName}>/$<TARGET_FILE_NAME:${TargetName}>"
        )
        install(
            DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
            DESTINATION ${InstallDestination}
        )
    endif()
endmacro()