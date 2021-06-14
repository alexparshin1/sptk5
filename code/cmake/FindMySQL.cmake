IF (WIN32)
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        $ENV{ProgramFiles}/MySQL/*
        $ENV{ProgramFiles}/MySQL/*/include
        $ENV{ProgramW6432}/MySQL/*
        $ENV{ProgramW6432}/MySQL/*/include
        )
   SET (MARIADB_POSSIBLE_INCLUDE_PATHS
        $ENV{ProgramW6432}/MariaDB/*
        $ENV{ProgramW6432}/MariaDB/include
        $ENV{ProgramFiles}/MariaDB/*
        $ENV{ProgramFiles}/MariaDB/*/include
        )
   SET (MYSQL_POSSIBLE_LIB_PATHS
        $ENV{ProgramFiles}/MySQL/*/lib
        $ENV{ProgramW6432}/MySQL/*/lib
        $ENV{ProgramW6432}/MariaDB/*/lib
        $ENV{ProgramFiles}/MariaDB/*/lib
        )
ELSE (WIN32)
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        /usr/include
        /usr/include/mysql
        /usr/mysql/include
        /usr/local/include
       )
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        /usr/include
        /usr/local/include
        /usr/include/mariadb
       )
   SET (MYSQL_POSSIBLE_LIB_PATHS
        /usr/lib64
        /usr/lib64/*
        /usr/lib
        /usr/lib/*
        /usr/local/lib64
        /usr/local/lib)
ENDIF (WIN32)

FIND_PATH(MariaDB_INCLUDE_DIR mysql.h ${MARIADB_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES mysql include)
IF (NOT MariaDB_INCLUDE_DIR)
    FIND_PATH(MariaDB_INCLUDE_DIR mysql.h ${MARIADB_POSSIBLE_INCLUDE_PATHS})
ENDIF()

IF (MariaDB_INCLUDE_DIR)
    SET (MARIADB_FLAG 1)
    SET (MySQL_INCLUDE_DIR "${MariaDB_INCLUDE_DIR}")
    FIND_LIBRARY(MySQL_LIBRARY NAMES mariadbclient PATHS ${MYSQL_POSSIBLE_LIB_PATHS} PATH_SUFFIXES mysql vs14 vs17)
ELSE()
    SET(MARIADB_FLAG 0)
    FIND_PATH(MySQL_INCLUDE_DIR mysql.h ${MYSQL_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES mysql include)
    IF (NOT MySQL_INCLUDE_DIR)
        FIND_PATH(MySQL_INCLUDE_DIR mysql.h ${MYSQL_POSSIBLE_INCLUDE_PATHS})
    ENDIF()
    FIND_LIBRARY(MySQL_LIBRARY NAMES mysqlclient_r mysqlclient PATHS ${MYSQL_POSSIBLE_LIB_PATHS} PATH_SUFFIXES mysql vs14 vs17)
ENDIF()

FILE (STRINGS "${MySQL_INCLUDE_DIR}/mysql.h" MySQL_has_my_bool_match REGEX my_bool)

SET (MySQL_has_my_bool "0")
IF (MySQL_has_my_bool_match)
    SET (MySQL_has_my_bool "1")
ENDIF ()

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
