IF (WIN32)
   SET (PGSQL_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/PostgreSQL/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/PostgreSQL/*/include
        $ENV{ProgramW6432}/*/include
        $ENV{ProgramW6432}/PostgreSQL/*/include)
   SET (PGSQL_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/PostgreSQL/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramFiles}/PostgreSQL/*/lib
        $ENV{ProgramW6432}/*/lib
        $ENV{ProgramW6432}/PostgreSQL/*/lib)
   FIND_PATH(PostgreSQL_INCLUDE_DIR libpq-fe.h ${PGSQL_POSSIBLE_INCLUDE_PATHS})
   FIND_LIBRARY(PostgreSQL_LIBRARY NAMES libpq pq PATHS ${PGSQL_POSSIBLE_LIB_PATHS})
ELSE (WIN32)
   FIND_PROGRAM (PGSQL_CONFIG pg_config PATHS /usr/bin /usr/postgres/*/bin /usr/local/bin)
   IF (PGSQL_CONFIG)
      EXEC_PROGRAM ("${PGSQL_CONFIG}" ARGS --includedir OUTPUT_VARIABLE PostgreSQL_INCLUDE_DIR)
      EXEC_PROGRAM ("${PGSQL_CONFIG}" ARGS --libdir OUTPUT_VARIABLE PostgreSQL_LIB_DIR)
   ENDIF (PGSQL_CONFIG)
   FIND_LIBRARY(PostgreSQL_LIBRARY NAMES pq PATHS "${PostgreSQL_LIB_DIR}" /usr/lib /usr/local/lib)
ENDIF (WIN32)

IF (PostgreSQL_INCLUDE_DIR AND PostgreSQL_LIBRARY)
   SET(PostgreSQL_FOUND TRUE)
ENDIF (PostgreSQL_INCLUDE_DIR AND PostgreSQL_LIBRARY)

IF (PostgreSQL_FOUND)
   IF (NOT PostgreSQL_FIND_QUIETLY)
      MESSAGE(STATUS "Found PostgreSQL: ${PostgreSQL_LIBRARY}")
   ENDIF (NOT PostgreSQL_FIND_QUIETLY)
ELSE (PostgreSQL_FOUND)
   IF (PostgreSQL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PostgreSQL")
   ENDIF (PostgreSQL_FIND_REQUIRED)
ENDIF (PostgreSQL_FOUND)

IF (PostgreSQL_INCLUDE_DIR AND PostgreSQL_LIBRARY)
   SET(PGSQL_FOUND TRUE)
ENDIF (PostgreSQL_INCLUDE_DIR AND PostgreSQL_LIBRARY)

IF (PGSQL_FOUND)
   IF (NOT PostgreSQL_FIND_QUIETLY)
      MESSAGE(STATUS "Found PostgreSQL: ${PostgreSQL_LIBRARY}")
   ENDIF (NOT PostgreSQL_FIND_QUIETLY)
ELSE (PGSQL_FOUND)
   IF (PostgreSQL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PostgreSQL")
   ENDIF (PostgreSQL_FIND_REQUIRED)
ENDIF (PGSQL_FOUND)
