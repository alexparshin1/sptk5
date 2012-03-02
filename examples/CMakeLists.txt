IF (NO_EXAMPLES)
    MESSAGE("Usage examples:     don't compile")
ELSE (NO_EXAMPLES)
    MESSAGE("Usage examples:     compile")

    ADD_SUBDIRECTORY (xml)
    ADD_SUBDIRECTORY (database)
    ADD_SUBDIRECTORY (threads)
    ADD_SUBDIRECTORY (utils)

    ADD_SUBDIRECTORY (datasources)
    ADD_SUBDIRECTORY (net)

    IF (FLTK_FLAG)
        ADD_SUBDIRECTORY (gui)
    ENDIF (FLTK_FLAG)

ENDIF (NO_EXAMPLES)
