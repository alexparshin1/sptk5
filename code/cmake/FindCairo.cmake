IF (WIN32)
   SET (CAIRO_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/*/inc
        $ENV{ProgramW6432}/*/include
        $ENV{ProgramW6432}/*/inc)
   SET (CAIRO_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib/x64
        $ENV{ProgramFiles}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (CAIRO_POSSIBLE_INCLUDE_PATHS
        /usr/local/include
        /usr/local/include/cairo
        /usr/include/cairo
        /usr/include)
   SET (CAIRO_POSSIBLE_LIB_PATHS
        /usr/local/lib
        /usr/lib /usr/lib/*)
ENDIF (WIN32)

FIND_PATH(CAIRO_INCLUDE_DIR cairo.h PATHS ${CAIRO_POSSIBLE_INCLUDE_PATHS})
FIND_LIBRARY(CAIRO_LIBRARY NAMES cairo PATHS ${CAIRO_POSSIBLE_LIB_PATHS})

IF (CAIRO_INCLUDE_DIR AND CAIRO_LIBRARY)
   SET(CAIRO_FOUND TRUE)
ENDIF (CAIRO_INCLUDE_DIR AND CAIRO_LIBRARY)

IF (CAIRO_FOUND)
   IF (NOT CAIRO_FIND_QUIETLY)
      MESSAGE(STATUS "Found CAIRO: ${CAIRO_LIBRARY}")
   ENDIF (NOT CAIRO_FIND_QUIETLY)
ELSE (CAIRO_FOUND)
   IF (CAIRO_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CAIRO")
   ENDIF (CAIRO_FIND_REQUIRED)
ENDIF (CAIRO_FOUND)
