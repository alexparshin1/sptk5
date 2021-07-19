FILE(GLOB SPUTIL_SOURCES core/*.cpp json/*.cpp jwt/*.cpp net/*.cpp xdoc/*.cpp tar/*.cpp tar/*.h threads/*.cpp xml/*.cpp)
SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} ${SPUTL_INCLUDE_FILES})

FILE(GLOB CONDITIONAL_SOURCES net/SocketPool.* core/ZLib.cpp core/Brotli.cpp core/RegularExpression.cpp)
LIST(REMOVE_ITEM SPUTIL_SOURCES ${CONDITIONAL_SOURCES})

IF (ZLIB_FOUND)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} core/ZLib.cpp)
ENDIF ()

IF (BROTLI_FLAG)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} core/Brotli.cpp)
ENDIF ()

IF (EPOLL_FLAG)
    IF (LINUX)
        MESSAGE("Socket events:      epoll (Linux)")
        SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.LinuxAndWindows.cpp)
    ENDIF ()

    IF (WIN32)
        MESSAGE("Socket events:      wepoll (Windows)")
        SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.LinuxAndWindows.cpp)
    ENDIF ()

    IF (BSD)
        MESSAGE("Socket events:      kqueue (BSD)")
        SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.BSD.cpp)
    ENDIF ()
ENDIF ()


IF (PCRE_FLAG OR PCRE2_FLAG)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} core/RegularExpression.cpp)
ENDIF ()

ADD_LIBRARY(sputil5 ${LIBRARY_TYPE} ${SPUTIL_SOURCES})
SET_TARGET_PROPERTIES(sputil5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

INSTALL(
    TARGETS sputil5
    RUNTIME DESTINATION bin COMPONENT core
    LIBRARY DESTINATION lib COMPONENT core
    ARCHIVE DESTINATION lib COMPONENT core)
