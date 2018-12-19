IF (WIN32)
   SET (ZMQ_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include
        $ENV{ProgramFiles}/*/include
        $ENV{ProgramW6432}/*/include)
   SET (ZMQ_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (ZMQ_POSSIBLE_INCLUDE_PATHS
        /usr/include
        /usr/zmq/include
        /usr/local/include
        /opt/zmq/include)
   SET (ZMQ_POSSIBLE_LIB_PATHS
        /usr/lib64
        /usr/lib
        /usr/lib/*
        /usr/local/lib
        /opt/zmq/lib)
ENDIF (WIN32)


FIND_PATH(ZMQ_INCLUDE_DIR ibase.h ${ZMQ_POSSIBLE_INCLUDE_PATHS} PATH_SUFFIXES zmq)
FIND_LIBRARY(ZMQ_LIBRARY NAMES zmq PATHS ${ZMQ_POSSIBLE_LIB_PATHS} PATH_SUFFIXES zmq)

IF (ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY)
   SET(ZMQ_FOUND TRUE)
ENDIF (ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY)

IF (ZMQ_FOUND)
   IF (NOT ZMQ_FIND_QUIETLY)
      MESSAGE(STATUS "Found ZeroMQ: ${ZMQ_LIBRARY}")
   ENDIF (NOT ZMQ_FIND_QUIETLY)
ELSE (ZMQ_FOUND)
   IF (ZMQ_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ZeroMQ")
   ENDIF (ZMQ_FIND_REQUIRED)
ENDIF (ZMQ_FOUND)
