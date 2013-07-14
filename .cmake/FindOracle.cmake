SET (ORACLE_HOME $ENV{ORACLE_HOME})

IF (WIN32)
   SET (ORACLE_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/*/include/oracle)
   SET (ORACLE_POSSIBLE_LIB_PATHS $ENV{SystemDrive}/*/lib $ENV{ProgramFiles}/*/lib)
ELSE (WIN32)
   SET (ORACLE_POSSIBLE_INCLUDE_PATHS
        ${ORACLE_HOME}/include
        ${ORACLE_HOME}/*/public
        /usr/include
        /usr/include/oracle/*/*
        /usr/local/include
        /usr/local/include/oracle/*/*)
   SET (ORACLE_POSSIBLE_LIB_PATHS ${ORACLE_HOME}/lib /usr/lib /usr/local/lib)
ENDIF (WIN32)

SET (Oracle_FOUND FALSE)

IF (ORACLE_HOME)
    FIND_PATH (Oracle_INCLUDE_DIRS occi.h ${ORACLE_POSSIBLE_INCLUDE_PATHS})
    IF (Oracle_INCLUDE_DIRS)
        FIND_LIBRARY (ORACLE_CLNTSH_LIBRARY clntsh ${ORACLE_POSSIBLE_LIB_PATHS})
        FIND_LIBRARY (ORACLE_OCCI_LIBRARY occi ${ORACLE_POSSIBLE_LIB_PATHS})
        SET (Oracle_LIBRARIES ${ORACLE_CLNTSH_LIBRARY} ${ORACLE_OCCI_LIBRARY})
    ENDIF (Oracle_INCLUDE_DIRS)
    IF (Oracle_LIBRARIES)
        SET (Oracle_FOUND TRUE)
        INCLUDE_DIRECTORIES(${Oracle_INCLUDE_DIRS})
        LINK_DIRECTORIES(${Oracle_LIBRARIES})
    ENDIF (Oracle_LIBRARIES)
ENDIF (ORACLE_HOME)

IF (Oracle_FOUND)
   IF (NOT Oracle_FIND_QUIETLY)
      MESSAGE(STATUS "Found Oracle: ${Oracle_LIBRARIES}")
   ENDIF (NOT Oracle_FIND_QUIETLY)
ELSE (Oracle_FOUND)
   IF (Oracle_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Oracle")
   ENDIF (Oracle_FIND_REQUIRED)
ENDIF (Oracle_FOUND)
