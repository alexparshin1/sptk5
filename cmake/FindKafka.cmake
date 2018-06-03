IF (WIN32)
   SET (KAFKA_POSSIBLE_INCLUDE_PATHS
        $ENV{SystemDrive}/*/include/librdkafka
        $ENV{ProgramFiles}/*/include/librdkafka
        $ENV{ProgramW6432}/*/include/librdkafka)
   SET (KAFKA_POSSIBLE_LIB_PATHS
        $ENV{SystemDrive}/*/lib/x64
        $ENV{ProgramFiles}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramFiles}/*/lib
        $ENV{ProgramW6432}/*/lib/x64
        $ENV{SystemDrive}/*/lib
        $ENV{ProgramW6432}/*/lib)
ELSE (WIN32)
   SET (KAFKA_POSSIBLE_INCLUDE_PATHS
        $ENV{HOME}/local/include/librdkafka
        /usr/local/include/librdkafka
        /usr/include/librdkafka)
   SET (KAFKA_POSSIBLE_LIB_PATHS
        $ENV{HOME}/local/lib
        /usr/local/lib
        /usr/lib /usr/lib/*)
ENDIF (WIN32)

FIND_PATH(KAFKA_INCLUDE_DIR rdkafkacpp.h ${KAFKA_POSSIBLE_INCLUDE_PATHS})
FIND_LIBRARY(KAFKA_LIBRARY NAMES rdkafka++ PATHS ${KAFKA_POSSIBLE_LIB_PATHS})

IF (KAFKA_INCLUDE_DIR AND KAFKA_LIBRARY)
   SET(KAFKA_FOUND TRUE)
ENDIF (KAFKA_INCLUDE_DIR AND KAFKA_LIBRARY)

IF (KAFKA_FOUND)
   IF (NOT KAFKA_FIND_QUIETLY)
      MESSAGE(STATUS "Found Apache Kafka: ${KAFKA_LIBRARY}")
   ENDIF (NOT KAFKA_FIND_QUIETLY)
ELSE (KAFKA_FOUND)
   IF (KAFKA_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Apache Kafka")
   ENDIF (KAFKA_FIND_REQUIRED)
ENDIF (KAFKA_FOUND)
