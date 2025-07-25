SET (DRIVER_LIBRARY_TYPE SHARED)

IF (DRIVER_LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_DRIVER_LIBRARY)
ENDIF(DRIVER_LIBRARY_TYPE STREQUAL "SHARED")

# ODBC support library
IF (ODBC_FLAG)
    ADD_LIBRARY (spdb5_odbc ${DRIVER_LIBRARY_TYPE} odbc/ODBC.cpp odbc/ODBCConnection.cpp)
    SET_TARGET_PROPERTIES(spdb5_odbc PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    TARGET_LINK_LIBRARIES(spdb5_odbc spdb5 sputil5 ${ODBC_LIBRARY})
    INSTALL(TARGETS spdb5_odbc RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF (ODBC_FLAG)

# SQLite3 support library
SET (SQLITE3_SOURCES)
IF (SQLITE3_FLAG)
    ADD_LIBRARY (spdb5_sqlite3 ${DRIVER_LIBRARY_TYPE} SQLite3/CSQLite3Connection.cpp)
    SET_TARGET_PROPERTIES(spdb5_sqlite3 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    TARGET_LINK_LIBRARIES(spdb5_sqlite3 spdb5 sputil5 ${SQLITE3_LIBRARY})
    INSTALL(TARGETS spdb5_sqlite3 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF (SQLITE3_FLAG)

SET (POSTGRESQL_SOURCES)
IF (POSTGRESQL_FLAG)
    ADD_LIBRARY (spdb5_postgresql ${DRIVER_LIBRARY_TYPE}
            PostgreSQL/PostgreSQLConnection.cpp
            PostgreSQL/PostgreSQLParamValues.cpp
        PostgreSQL/htonq.cpp
    )
    SET_TARGET_PROPERTIES(spdb5_postgresql PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    INCLUDE_DIRECTORIES (${PostgreSQL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(spdb5_postgresql spdb5 sputil5 ${PostgreSQL_LIBRARY})
    INSTALL(TARGETS spdb5_postgresql RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF (POSTGRESQL_FLAG)

IF (ORACLE_FLAG)
    ADD_LIBRARY (spdb5_oracle ${DRIVER_LIBRARY_TYPE}
            Oracle/OracleEnvironment.cpp
            Oracle/OracleStatement.cpp
            Oracle/OracleConnection.cpp
            Oracle/OracleBulkInsertQuery.cpp
    )

    # Oracle uses old C++ ABI
    #TARGET_COMPILE_OPTIONS (spdb5_oracle PUBLIC -D_GLIBCXX_USE_CXX11_ABI=0)

    SET_TARGET_PROPERTIES(spdb5_oracle PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    TARGET_LINK_LIBRARIES (spdb5_oracle spdb5 sputil5 ${Oracle_LIBRARIES})
    INSTALL(TARGETS spdb5_oracle RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF (ORACLE_FLAG)

IF (MYSQL_FLAG)
    ADD_LIBRARY (spdb5_mysql ${DRIVER_LIBRARY_TYPE}
            MySQL/MySQLStatement.cpp
            MySQL/MySQLConnection.cpp
    )
    SET_TARGET_PROPERTIES(spdb5_mysql PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    INCLUDE_DIRECTORIES (${MySQL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES (spdb5_mysql spdb5 sputil5 ${MySQL_LIBRARY})
    INSTALL(TARGETS spdb5_mysql RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF (MYSQL_FLAG)

IF (FIREBIRD_FLAG)
    ADD_LIBRARY (spdb5_firebird ${LIBRARY_TYPE}
        Firebird/FirebirdConnection.cpp
        Firebird/FirebirdStatement.cpp
    )
    SET_TARGET_PROPERTIES(spdb5_firebird PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    INCLUDE_DIRECTORIES (${Firebird_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES (spdb5_firebird spdb5 sputil5 ${Firebird_LIBRARY})
    INSTALL(TARGETS spdb5_firebird RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ENDIF (FIREBIRD_FLAG)
