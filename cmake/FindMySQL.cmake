IF (WIN32)
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        $ENV{ProgramFiles}/MySQL/*
        $ENV{ProgramFiles}/MySQL/*/include
        $ENV{ProgramW6432}/MySQL/*/include)
   SET (MYSQL_POSSIBLE_LIB_PATHS
        $ENV{ProgramFiles}/MySQL/*/lib
        $ENV{ProgramW6432}/MySQL/*/lib)
ELSE (WIN32)
   SET (MYSQL_POSSIBLE_INCLUDE_PATHS
        /usr/include
        /usr/include/mysql
        /usr/mysql/include
        /usr/local/include)
   SET (MYSQL_POSSIBLE_LIB_PATHS
        /usr/lib64
        /usr/lib
        /usr/lib/*
        /usr/local/lib)
ENDIF (WIN32)


FIND_PATH(MySQL_INCLUDE_DIR mysql.h ${MYSQL_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES mysql include)
#FIND_PATH(MySQL_INCLUDE_DIR mysql.h ${MYSQL_POSSIBLE_INCLUDE_PATHS})
FIND_LIBRARY(MySQL_LIBRARY NAMES mysqlclient_r mysqlclient PATHS ${MYSQL_POSSIBLE_LIB_PATHS} PATH_SUFFIXES mysql vs14 vs17)

IF (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)
   SET(MySQL_FOUND TRUE)
ENDIF (MySQL_INCLUDE_DIR AND MySQL_LIBRARY)

IF (MySQL_FOUND)
   IF (NOT MySQL_FIND_QUIETLY)
      MESSAGE(STATUS "Found MySQL: ${MySQL_LIBRARY}")
   ENDIF (NOT MySQL_FIND_QUIETLY)
ELSE (MySQL_FOUND)
   IF (MySQL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find MySQL")
   ENDIF (MySQL_FIND_REQUIRED)
ENDIF (MySQL_FOUND)
