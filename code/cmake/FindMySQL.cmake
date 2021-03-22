IF (WIN32)
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        $ENV{ProgramFiles}/MySQL/*
        $ENV{ProgramW6432}/MySQL/*/include
        $ENV{ProgramW6432}/MySQL/*)
   SET (MYSQL_POSSIBLE_LIB_PATHS
        $ENV{ProgramFiles}/MySQL/*/lib
        $ENV{ProgramW6432}/MySQL/*/lib)
ELSE (WIN32)
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        /usr/include
        /usr/include/mysql
        /usr/mysql/include
        /usr/local/include
        /usr/include/mariadb
       )
   SET (MYSQL_POSSIBLE_LIB_PATHS
        /usr/lib64
        /usr/lib
        /usr/lib/*
        /usr/local/lib)
ENDIF (WIN32)


FIND_PATH(MySQL_INCLUDE_DIR mysql.h ${MYSQL_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES mysql include)
IF (NOT MySQL_INCLUDE_DIR)
    FIND_PATH(MySQL_INCLUDE_DIR mysql.h ${MYSQL_POSSIBLE_INCLUDE_PATHS})
    FIND_PATH(MariaDB_INCLUDE_DIR mariadb_version.h ${MYSQL_POSSIBLE_INCLUDE_PATHS})
ELSE()
    FIND_PATH(MariaDB_INCLUDE_DIR mariadb_version.h ${MYSQL_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES mysql include)
ENDIF()

IF (MariaDB_INCLUDE_DIR)
    SET (MARIADB_FLAG 1)
    IF (NOT WIN32)
        FILE (STRINGS "${MariaDB_INCLUDE_DIR}/mysql.h" MySQL_has_my_bool_match REGEX my_bool)
    ENDIF ()
ELSE ()
    SET(MARIADB_FLAG 0)
    IF (NOT WIN32)
        FILE (STRINGS "${MySQL_INCLUDE_DIR}/mysql.h" MySQL_has_my_bool_match REGEX my_bool)
    ENDIF ()
ENDIF ()

SET (MySQL_has_my_bool "0")
IF (MySQL_has_my_bool_match)
    SET (MySQL_has_my_bool "1")
ENDIF ()

FIND_LIBRARY(MySQL_LIBRARY NAMES mysqlclient_r mysqlclient mariadbclient PATHS ${MYSQL_POSSIBLE_LIB_PATHS} PATH_SUFFIXES mysql vs14 vs17)

IF (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)
   SET(MySQL_FOUND TRUE)
ENDIF (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)

IF (MySQL_FOUND)
   IF (NOT MySQL_FIND_QUIETLY)
      IF (MariaDB_FLAG)
         MESSAGE(STATUS "Found MariaDB client: ${MySQL_LIBRARY}")
      ELSE ()
         MESSAGE(STATUS "Found MySQL client: ${MySQL_LIBRARY}")
      ENDIF ()
   ENDIF ()
ELSE (MySQL_FOUND)
   IF (MySQL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find MySQL or MariaDB client")
   ENDIF (MySQL_FIND_REQUIRED)
ENDIF (MySQL_FOUND)
