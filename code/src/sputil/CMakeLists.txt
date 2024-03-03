FILE(GLOB SPUTIL_SOURCES core/*.cpp jwt/*.cpp net/*.cpp xdoc/*.cpp tar/*.cpp tar/*.h threads/*.cpp)
FILE(GLOB SPUTIL_INCLUDE_FILES 
    ${SPTK_INCLUDE_DIR}/*.h 
    ${SPTK_INCLUDE_DIR}/jwt/*.h
    ${SPTK_INCLUDE_DIR}/net*.h
    ${SPTK_INCLUDE_DIR}/xdoc/*.h
    ${SPTK_INCLUDE_DIR}/tar/*.h
    ${SPTK_INCLUDE_DIR}/threads/*.h
)

SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} ${SPUTL_INCLUDE_FILES})

FILE(GLOB CONDITIONAL_SOURCES net/SocketPool.* core/ZLib.cpp core/Brotli.cpp core/RegularExpression.cpp)
LIST(REMOVE_ITEM SPUTIL_SOURCES ${CONDITIONAL_SOURCES})

IF (WIN32)
    FILE(GLOB WEPOLL_SOURCES wepoll/*.c wepoll/*.h)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} ${WEPOLL_SOURCES})
ENDIF ()

IF (ZLIB_FOUND)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} core/ZLib.cpp)
ENDIF ()

IF (BROTLI_FLAG)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} core/Brotli.cpp)
ENDIF ()

IF (USE_EPOLL)
    IF (EPOLL_FOUND)
        MESSAGE("Socket events:      epoll")
        SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.cpp net/SocketPool.LinuxAndWindows.cpp)
    ELSE ()

        IF (WIN32)
            MESSAGE("Socket events:      wepoll")
            SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.cpp net/SocketPool.LinuxAndWindows.cpp)
        ENDIF ()

        IF (BSD)
            MESSAGE("Socket events:      kqueue")
            SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.cpp net/SocketPool.BSD.cpp)
        ENDIF ()

    ENDIF ()
ENDIF ()


IF (PCRE_FLAG OR PCRE2_FLAG)
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} core/RegularExpression.cpp)
ENDIF ()

ADD_LIBRARY(sputil5 ${LIBRARY_TYPE} ${SPUTIL_SOURCES})
SET_TARGET_PROPERTIES(sputil5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

IF (SET_RPATH)
    SET_TARGET_PROPERTIES(sputil5 PROPERTIES INSTALL_RPATH "" )
ENDIF ()

INSTALL(
    TARGETS sputil5
    RUNTIME DESTINATION bin COMPONENT core
    LIBRARY DESTINATION lib COMPONENT core
    ARCHIVE DESTINATION lib COMPONENT core)
