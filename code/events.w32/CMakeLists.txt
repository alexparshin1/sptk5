IF (WIN32)

    # Generating event_provider.rc and event_provider.h from the message definitions.
    # The generated files are also checked in, so the message compiler is only needed
    # when event_provider.mc changes.
    IF (MC_COMPILER)
        ADD_CUSTOM_COMMAND (
            OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/event_provider.rc
                    ${CMAKE_CURRENT_SOURCE_DIR}/event_provider.h
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/event_provider.mc
            COMMAND ${MC_COMPILER}
            ARGS -A ${CMAKE_CURRENT_SOURCE_DIR}/event_provider.mc -h ${CMAKE_CURRENT_SOURCE_DIR} -r ${CMAKE_CURRENT_SOURCE_DIR}
        )
    ENDIF (MC_COMPILER)

    # Build target for event_provider.rc
    ADD_CUSTOM_TARGET (
        "Windows_Event_Log_Resources" ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/event_provider.rc
    )

    # Make resource files group
    FILE (
        GLOB resourceFiles
        ${CMAKE_CURRENT_SOURCE_DIR}/README
        ${CMAKE_CURRENT_SOURCE_DIR}/event_provider.rc
        ${CMAKE_CURRENT_SOURCE_DIR}/MSG00409.bin
    )

    # Install resource files into SPTK share directory
    INSTALL (
        FILES ${resourceFiles}
        DESTINATION share/sptk5/resources COMPONENT core)

ENDIF (WIN32)
