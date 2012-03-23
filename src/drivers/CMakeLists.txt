# ODBC support library
IF (ODBC_FLAG)
    ADD_LIBRARY (spdb5_odbc ${LIBRARY_TYPE} odbc/CODBC.cpp odbc/CODBCDatabase.cpp)
    #TARGET_LINK_LIBRARIES(spdb5_odbc spdb5 sputl5)
ENDIF (ODBC_FLAG)
