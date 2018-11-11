IF (WIN32)
   SET (FIREBIRD_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramW6432}/*/include)
   SET (FIREBIRD_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (FIREBIRD_POSSIBLE_INCLUDE_PATHS
        /usr/include
        /usr/firebird/include
        /usr/local/include
        /opt/firebird/include)
   SET (FIREBIRD_POSSIBLE_LIB_PATHS
        /usr/lib64
        /usr/lib
        /usr/lib/*
        /usr/local/lib
        /opt/firebird/lib)
ENDIF (WIN32)


FIND_PATH(Firebird_INCLUDE_DIR ibase.h ${FIREBIRD_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES firebird)
FIND_LIBRARY(Firebird_LIBRARY NAMES fbclient PATHS ${FIREBIRD_POSSIBLE_LIB_PATHS} PATH_SUFFIXES firebird)

IF (Firebird_INCLUDE_DIR AND Firebird_LIBRARY)
   SET(Firebird_FOUND TRUE)
ENDIF (Firebird_INCLUDE_DIR AND Firebird_LIBRARY)

IF (Firebird_FOUND)
   IF (NOT Firebird_FIND_QUIETLY)
      MESSAGE(STATUS "Found Firebird: ${Firebird_LIBRARY}")
   ENDIF (NOT Firebird_FIND_QUIETLY)
ELSE (Firebird_FOUND)
   IF (Firebird_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Firebird")
   ENDIF (Firebird_FIND_REQUIRED)
ENDIF (Firebird_FOUND)
