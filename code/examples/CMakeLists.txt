# Placing example executables to bin subdirectory
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

IF (NO_EXAMPLES)
    MESSAGE("Usage examples:     don't compile")
ELSE (NO_EXAMPLES)
    MESSAGE("Usage examples:     compile")

    ADD_SUBDIRECTORY(jwt)
    ADD_SUBDIRECTORY(database)
    ADD_SUBDIRECTORY(core)

    IF (FLTK_FLAG)
        ADD_SUBDIRECTORY(gui)
    ENDIF (FLTK_FLAG)

ENDIF (NO_EXAMPLES)
