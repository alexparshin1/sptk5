IF (WIN32)

    # Generating events.rc to be included to any Visual Studio projects
    # that use Windows event log.
    ADD_CUSTOM_COMMAND (
        OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/events.rc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/events.mc
        COMMAND ${MC_COMPILER}
        ARGS -A ${CMAKE_CURRENT_SOURCE_DIR}/events.mc -h ${CMAKE_CURRENT_SOURCE_DIR} -r ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Build target for events.rc
    ADD_CUSTOM_TARGET (
        "Windows_Event_Log_Resources" ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/events.rc
    )

    # Make resource files group
    FILE (
        GLOB resourceFiles
        ${CMAKE_CURRENT_SOURCE_DIR}/README
        ${CMAKE_CURRENT_SOURCE_DIR}/events.rc
        ${CMAKE_CURRENT_SOURCE_DIR}/MSG00409.bin
    )

    # Install resource files into SPTK share directory
    INSTALL (FILES ${resourceFiles} DESTINATION share/sptk5/resources)

ENDIF (WIN32)
