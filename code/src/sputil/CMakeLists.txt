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

    # Windows event log message table. It's linked into this library, and SysLogEngine
    # registers this library as the event source EventMessageFile, so applications using
    # SPTK don't have to embed the resource into their own executables.
    SET(SPUTIL_SOURCES ${SPUTIL_SOURCES} ${SPTK_SOURCE_DIR}/events.w32/event_provider.rc)
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

ADD_LIBRARY(sputil5 SHARED ${SPUTIL_SOURCES})
SET_SPTK_TARGET_PROPERTIES(sputil5)

IF (WIN32)
    # event_provider.rc and event_provider.h are generated from event_provider.mc
    ADD_DEPENDENCIES(sputil5 Windows_Event_Log_Resources)
ENDIF ()

SET_SPTK_TARGET_PROPERTIES(sputil5)

INSTALL(TARGETS sputil5
        RUNTIME DESTINATION bin COMPONENT core
        LIBRARY DESTINATION lib COMPONENT core
        ARCHIVE DESTINATION lib COMPONENT core)
