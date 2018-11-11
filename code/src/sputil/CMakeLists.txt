IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

SET (SPUTIL_SOURCES
    core/Base64.cpp core/Crypt.cpp core/LogEngine.cpp
    core/Buffer.cpp core/DataSource.cpp core/DateTime.cpp core/Exception.cpp core/CommandLine.cpp
    core/Field.cpp core/FieldList.cpp core/FileLogEngine.cpp core/IntList.cpp core/Registry.cpp core/SharedStrings.cpp
    core/String.cpp core/Strings.cpp core/SysLogEngine.cpp core/UniqueInstance.cpp core/Variant.cpp core/string_ext.cpp
    core/DirectoryDS.cpp core/MemoryDS.cpp core/Logger.cpp core/SystemException.cpp core/md5.cpp
    json/JsonArrayData.cpp json/JsonObjectData.cpp json/JsonDocument.cpp json/JsonElement.cpp json/JsonParser.cpp
    jwt/JWT.cpp jwt/JWT-openssl.cpp
    net/AsyncTcpSocket.cpp net/BaseMailConnect.cpp net/BaseSocket.cpp net/CachedSSLContext.cpp
    net/Host.cpp net/HttpAuthentication.cpp net/HttpConnect.cpp net/HttpParams.cpp net/ImapConnect.cpp net/MailMessageBody.cpp
    net/SmtpConnect.cpp net/SSLContext.cpp net/SSLSocket.cpp net/SocketEvents.cpp
    net/TCPServer.cpp net/TCPServerListener.cpp net/TCPSocket.cpp net/ServerConnection.cpp
    net/UDPSocket.cpp net/ImapDS.cpp
    xml/Attributes.cpp xml/Document.cpp xml/DocType.cpp xml/Node.cpp xml/NodeList.cpp xml/Value.cpp
    tar/block.cpp tar/Tar.cpp tar/decode.cpp tar/handle.cpp tar/libtar_hash.cpp tar/libtar_list.cpp tar/util.cpp
    threads/RWLock.cpp threads/Locks.cpp threads/Thread.cpp threads/ThreadPool.cpp
    threads/Semaphore.cpp threads/Runable.cpp threads/WorkerThread.cpp threads/Timer.cpp
    )

IF (ZLIB_FOUND)
    SET (SPUTIL_SOURCES ${SPUTIL_SOURCES} core/ZLib.cpp)
ENDIF ()

IF (LINUX)
    MESSAGE ("Socket events:      epoll (Linux)")
    SET (SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.Linux.cpp)
ENDIF ()

IF (WIN32)
    MESSAGE ("Socket events:      WSAAsyncSelect (Windows)")
    SET (SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.Windows.cpp)
ENDIF ()

IF (BSD)
    MESSAGE ("Socket events:      kqueue (BSD)")
    SET (SPUTIL_SOURCES ${SPUTIL_SOURCES} net/SocketPool.BSD.cpp)
ENDIF ()


IF (PCRE_FLAG)
    SET (SPUTIL_SOURCES ${SPUTIL_SOURCES} core/RegularExpression.cpp net/HttpReader.cpp net/HttpAuthentication.cpp threads/Locks.cpp)
ENDIF (PCRE_FLAG)

ADD_LIBRARY (sputil5 ${LIBRARY_TYPE} ${SPUTIL_SOURCES})
SET_TARGET_PROPERTIES(sputil5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

INSTALL(TARGETS sputil5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
