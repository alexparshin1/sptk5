# SPDB library

IF (LIBRARY_TYPE STREQUAL "SHARED")
    ADD_DEFINITIONS(-DSP_LIBRARY)
ENDIF (LIBRARY_TYPE STREQUAL "SHARED")

FILE(GLOB SPDB_SOURCE_FILES *.cpp)

ADD_LIBRARY(spdb5 SHARED ${SPDB_SOURCE_FILES} ${SPDB_INCLUDE_FILES})
SET_SPTK_TARGET_PROPERTIES(spdb5)
TARGET_LINK_LIBRARIES(spdb5 sputil5)

# The soname version the database drivers are built with, handed to spdb5 by the build rather than
# read from sptk-config.h, so the driver it opens is always the one built beside it. Taken from the
# header it went wrong on the first try: a stale generated sptk-config.h in the source tree shadowed
# the build tree's, a 5.6.11 libspdb5 opened libspdb5_sqlite3.so.5.6.10, that pulled in a second
# libspdb5 and libsputil5, and the process died in a double free at exit.
TARGET_COMPILE_DEFINITIONS(spdb5 PRIVATE SPTK_DRIVER_SONAME_VERSION="${SOVERSION}")

INSTALL(TARGETS spdb5
        RUNTIME DESTINATION bin COMPONENT db
        LIBRARY DESTINATION lib COMPONENT db
        ARCHIVE DESTINATION lib COMPONENT db)
