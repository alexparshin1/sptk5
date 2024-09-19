IF (WIN32)
   SET (OCI_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramFiles}/*/inc
        $ENV{ProgramW6432}/*/include
        $ENV{ProgramW6432}/*/inc)
   SET (OCI_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib/x64
        $ENV{ProgramFiles}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (OCI_POSSIBLE_INCLUDE_PATHS
        $ENV{HOME}/local/include
        /usr/local/include
        /usr/include)
   SET (OCI_POSSIBLE_LIB_PATHS
        $ENV{HOME}/local/lib
        /usr/local/lib
        /usr/lib /usr/lib/*)
ENDIF (WIN32)

FIND_PATH(OCI_INCLUDE_DIR ocilibcpp/core.hpp ${OCI_POSSIBLE_INCLUDE_PATHS})
FIND_LIBRARY(OCI_LIBRARY NAMES ocilib_static PATHS ${OCI_POSSIBLE_LIB_PATHS})
IF (NOT OCI_LIBRARY)
   FIND_LIBRARY(OCI_LIBRARY NAMES ocilib PATHS ${OCI_POSSIBLE_LIB_PATHS})
ENDIF ()

IF (OCI_INCLUDE_DIR AND OCI_LIBRARY)
   SET(OCI_FOUND TRUE)
ENDIF (OCI_INCLUDE_DIR AND OCI_LIBRARY)

IF (OCI_FOUND)
   IF (NOT OCI_FIND_QUIETLY)
      MESSAGE(STATUS "Found OCI: ${OCI_LIBRARY}")
   ENDIF (NOT OCI_FIND_QUIETLY)
ELSE (OCI_FOUND)
   IF (OCI_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find OCI")
   ENDIF (OCI_FIND_REQUIRED)
ENDIF (OCI_FOUND)
