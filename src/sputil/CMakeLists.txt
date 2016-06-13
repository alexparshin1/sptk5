IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

SET (SPUTIL_SOURCES
    core/Base64.cpp core/LogEngine.cpp
    core/CBuffer.cpp core/CDataSource.cpp core/DateTime.cpp core/Exception.cpp core/CCommandLine.cpp
    core/CField.cpp core/CFieldList.cpp core/FileLogEngine.cpp core/CIntList.cpp core/CRegistry.cpp core/CSharedStrings.cpp
    core/CStrings.cpp core/SysLogEngine.cpp core/CUniqueInstance.cpp core/Variant.cpp core/string_ext.cpp
    core/CDirectoryDS.cpp core/CMemoryDS.cpp core/Logger.cpp core/CSystemException.cpp
    core/md5.cpp
    net/BaseMailConnect.cpp net/BaseSocket.cpp net/CGI.cpp net/FTPConnect.cpp
    net/HttpConnect.cpp net/HttpParams.cpp net/ImapConnect.cpp net/MailMessageBody.cpp
    net/SmtpConnect.cpp net/SSLContext.cpp net/SSLSocket.cpp
    net/TCPServer.cpp net/TCPSocket.cpp net/ServerConnection.cpp
    net/UDPSocket.cpp net/FtpDS.cpp net/ImapDS.cpp
    xml/CXmlAttributes.cpp xml/CXmlDoc.cpp xml/CXmlDocType.cpp xml/CXmlNode.cpp xml/CXmlNodeList.cpp xml/CXmlValue.cpp
    tar/block.cpp tar/CTar.cpp tar/decode.cpp tar/handle.cpp tar/libtar_hash.cpp tar/libtar_list.cpp tar/util.cpp
    threads/CRWLock.cpp threads/CSynchronized.cpp threads/CThread.cpp threads/CThreadPool.cpp
    threads/CSemaphore.cpp threads/CRunable.cpp threads/CWorkerThread.cpp
    )

IF (PCRE_FLAG)
    SET (SPUTIL_SOURCES ${SPUTIL_SOURCES} core/CRegExp.cpp)
ENDIF (PCRE_FLAG)

ADD_LIBRARY (sputil5 ${LIBRARY_TYPE} ${SPUTIL_SOURCES})

INSTALL(TARGETS sputil5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
