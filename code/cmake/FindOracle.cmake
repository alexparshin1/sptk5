SET (ORACLE_HOME $ENV{ORACLE_HOME})

IF (WIN32)
   SET (ORACLE_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/*/include/oracle
        $ENV{ProgramW6432}/*/include
        $ENV{ProgramW6432}/*/include/oracle)
   SET (ORACLE_POSSIBLE_LIB_PATHS $ENV{SystemDrive}/*/lib $ENV{ProgramFiles}/*/lib $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (ORACLE_POSSIBLE_INCLUDE_PATHS
        ${ORACLE_HOME}/include
        ${ORACLE_HOME}/*/public
        ${ORACLE_HOME}/sdk/include
        /opt/*/sdk/include
        /opt/oracle/*/sdk/include
        /usr/include
        /usr/include/oracle/*/*
        /usr/local/include
        /usr/local/include/oracle/*/*)
   SET (ORACLE_POSSIBLE_LIB_PATHS
        ${ORACLE_HOME}/lib
        ${ORACLE_HOME}/lib64
        ${ORACLE_HOME}
        /opt/*
        /opt/oracle/*
        /usr/lib
        /usr/lib/oracle/*/*/lib
        /usr/*/lib)
ENDIF (WIN32)

SET (Oracle_FOUND FALSE)

#IF (ORACLE_HOME)
    FIND_PATH (Oracle_INCLUDE_DIRS occi.h ${ORACLE_POSSIBLE_INCLUDE_PATHS})
    IF (Oracle_INCLUDE_DIRS)
        FIND_LIBRARY (ORACLE_CLNTSH_LIBRARY clntsh ${ORACLE_POSSIBLE_LIB_PATHS})
        FIND_LIBRARY (ORACLE_OCCI_LIBRARY occi ${ORACLE_POSSIBLE_LIB_PATHS})
        SET (Oracle_LIBRARIES ${ORACLE_CLNTSH_LIBRARY} ${ORACLE_OCCI_LIBRARY})
    ENDIF (Oracle_INCLUDE_DIRS)
    IF (Oracle_LIBRARIES)
        SET (Oracle_FOUND TRUE)
        INCLUDE_DIRECTORIES(${Oracle_INCLUDE_DIRS})
    ENDIF (Oracle_LIBRARIES)
#ENDIF (ORACLE_HOME)

IF (Oracle_FOUND)
   IF (NOT Oracle_FIND_QUIETLY)
      MESSAGE(STATUS "Found Oracle: ${Oracle_LIBRARIES}")
   ENDIF (NOT Oracle_FIND_QUIETLY)
ELSE (Oracle_FOUND)
   IF (Oracle_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Oracle")
   ENDIF (Oracle_FIND_REQUIRED)
ENDIF (Oracle_FOUND)
