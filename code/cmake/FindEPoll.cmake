IF (WIN32)

   SET (EPOLL_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        "$ENV{ProgramFiles} (x86)/*/include"
        $ENV{ProgramW6432}/*/include)

   SET (EPOLL_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        "$ENV{ProgramFiles} (x86)/*/lib"
        $ENV{ProgramW6432}/*/lib)

   SET (EPOLL_NAME wepoll)

   FIND_PATH(EPOLL_INCLUDE_DIR wepoll.h ${EPOLL_POSSIBLE_INCLUDE_PATHS})

   FIND_LIBRARY(EPOLL_LIBRARY NAMES wepoll.lib PATHS ${EPOLL_POSSIBLE_LIB_PATHS})

ELSE (WIN32)

   SET (EPOLL_POSSIBLE_INCLUDE_PATHS /usr/include)

   SET (EPOLL_POSSIBLE_LIB_PATHS /usr/lib /usr/local/lib)

   FIND_PATH(EPOLL_INCLUDE_DIR sys/epoll.h ${EPOLL_POSSIBLE_INCLUDE_PATHS})

   FIND_LIBRARY(EPOLL_LIBRARY NAMES epoll PATHS ${EPOLL_POSSIBLE_LIB_PATHS})

   SET (EPOLL_NAME epoll)

ENDIF (WIN32)

IF (EPOLL_INCLUDE_DIR AND EPOLL_LIBRARY)
   SET(EPOLL_FOUND TRUE)
ENDIF (EPOLL_INCLUDE_DIR AND EPOLL_LIBRARY)

IF (EPOLL_FOUND)
   IF (NOT EPoll_FIND_QUIETLY)
      MESSAGE(STATUS "Found EPoll:  ${EPOLL_LIBRARY}")
   ENDIF ()
ELSE ()
   IF (EPoll_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ${EPOLL_NAME} library")
   ENDIF ()
ENDIF ()
