INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES (/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

LINK_DIRECTORIES (${SPTK_BINARY_DIR})

LINK_LIBRARIES (sputil5 ${SOCKET_LIBRARY})
IF (UNIX)
    LINK_LIBRARIES (pthread rt)
ENDIF (UNIX)

ADD_EXECUTABLE (bulk_insert_test bulk_insert_test.cpp)
TARGET_LINK_LIBRARIES (bulk_insert_test sputil5 spdb5)

IF (SQLITE3_LIBRARY)
  ADD_EXECUTABLE (sqlite3_test sqlite3_test.cpp)
  TARGET_LINK_LIBRARIES (sqlite3_test sputil5 spdb5)

  INSTALL(TARGETS sqlite3_test
    RUNTIME DESTINATION share/sptk5/bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
ENDIF (SQLITE3_LIBRARY)

IF (ODBC_LIBRARY)
  #SET (ODBC_TESTS odbc_test odbc_test2 odbc_test3)
  SET (ODBC_TESTS driver_load_test odbc_test)
  LINK_LIBRARIES (spdb5)
  FOREACH (ODBC_TEST ${ODBC_TESTS})
    ADD_EXECUTABLE (${ODBC_TEST} ${ODBC_TEST}.cpp)
    TARGET_LINK_LIBRARIES (${ODBC_TEST} sputil5 spdb5)
  ENDFOREACH (ODBC_TEST)

  INSTALL(TARGETS ${ODBC_TESTS}
    RUNTIME DESTINATION lib/sptk5/bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
ENDIF (ODBC_LIBRARY)

ADD_EXECUTABLE (all_drivers_test all_drivers_test.cpp)
TARGET_LINK_LIBRARIES (all_drivers_test sputil5 spdb5)
