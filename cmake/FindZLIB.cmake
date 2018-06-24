IF (WIN32)
   SET (ZLIB_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/*/inc
        $ENV{ProgramW6432}/*/include
        $ENV{ProgramW6432}/*/inc)
   SET (ZLIB_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib/x64
        $ENV{ProgramFiles}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (ZLIB_POSSIBLE_INCLUDE_PATHS
        $ENV{HOME}/local/include
        /usr/local/include
        /usr/include)
   SET (ZLIB_POSSIBLE_LIB_PATHS
        $ENV{HOME}/local/lib
        /usr/local/lib
        /usr/lib /usr/lib/*)
ENDIF (WIN32)

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h ${ZLIB_POSSIBLE_INCLUDE_PATHS})
FIND_LIBRARY(ZLIB_LIBRARY NAMES z zlib PATHS ${ZLIB_POSSIBLE_LIB_PATHS})

IF (ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)
   SET(ZLIB_FOUND TRUE)
ENDIF (ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)

IF (ZLIB_FOUND)
   IF (NOT ZLIB_FIND_QUIETLY)
      MESSAGE(STATUS "Found ZLIB: ${ZLIB_LIBRARY}")
   ENDIF (NOT ZLIB_FIND_QUIETLY)
ELSE (ZLIB_FOUND)
   IF (ZLIB_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ZLIB")
   ENDIF (ZLIB_FIND_REQUIRED)
ENDIF (ZLIB_FOUND)
