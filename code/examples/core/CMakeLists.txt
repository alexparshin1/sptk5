INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES (/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

LINK_DIRECTORIES (${SPTK_BINARY_DIR})

LINK_LIBRARIES (sputil5 ${SOCKET_LIBRARY})
IF (UNIX)
    LINK_LIBRARIES (pthread rt)
ENDIF (UNIX)

SET (CONSOLE_TESTS
     command_line datetime exceptions logfile_test syslog_test
     unique_instance registry string2md5 encrypt_decrypt timer
     zlib_test
    )

IF (WIN32)
    SET (CONSOLE_TESTS ${CONSOLE_TESTS} eventlog_test)
ENDIF ()

IF (PCRE_FLAG)
    SET (CONSOLE_TESTS ${CONSOLE_TESTS} regexp)
ENDIF (PCRE_FLAG)

FOREACH (CONSOLE_TEST ${CONSOLE_TESTS})
    IF (UNIX)
        ADD_EXECUTABLE (${CONSOLE_TEST} ${CONSOLE_TEST}.cpp)
    ELSE (UNIX)
        ADD_EXECUTABLE (${CONSOLE_TEST} ${CONSOLE_TEST}.cpp ${CMAKE_SOURCE_DIR}/events.w32/event_provider.rc)
    ENDIF (UNIX)

ENDFOREACH (CONSOLE_TEST)

INSTALL(TARGETS ${CONSOLE_TESTS}
    RUNTIME DESTINATION lib/sptk5/bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
