IF (WIN32)
   SET (PCRE2_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/*/inc
        $ENV{ProgramW6432}/*/include
        $ENV{ProgramW6432}/*/inc)
   SET (PCRE2_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib/x64
        $ENV{ProgramFiles}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (PCRE2_POSSIBLE_INCLUDE_PATHS
        $ENV{HOME}/local/include
        /usr/local/include
        /usr/include)
   SET (PCRE2_POSSIBLE_LIB_PATHS
        $ENV{HOME}/local/lib
        /usr/local/lib
        /usr/lib /usr/lib/*)
ENDIF (WIN32)

FIND_PATH(PCRE2_INCLUDE_DIR pcre.h ${PCRE2_POSSIBLE_INCLUDE_PATHS})
FIND_LIBRARY(PCRE2_LIBRARY NAMES pcre2-8 PATHS ${PCRE2_POSSIBLE_LIB_PATHS})

IF (PCRE2_INCLUDE_DIR AND PCRE2_LIBRARY)
   SET(PCRE2_FOUND TRUE)
ENDIF (PCRE2_INCLUDE_DIR AND PCRE2_LIBRARY)

IF (PCRE2_FOUND)
   IF (NOT PCRE2_FIND_QUIETLY)
      MESSAGE(STATUS "Found PCRE2: ${PCRE2_LIBRARY}")
   ENDIF (NOT PCRE2_FIND_QUIETLY)
ELSE (PCRE2_FOUND)
   IF (PCRE2_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find PCRE2")
   ENDIF (PCRE2_FIND_REQUIRED)
ENDIF (PCRE2_FOUND)
