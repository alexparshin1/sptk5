IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

SET (SPUTIL_SOURCES
    core/CBase64.cpp core/CBaseLog.cpp 
    core/CBuffer.cpp core/CDataSource.cpp core/CDateTime.cpp core/CException.cpp core/CCommandLine.cpp
    core/CField.cpp core/CFieldList.cpp core/CFileLog.cpp core/CIntList.cpp core/CRegistry.cpp core/CSharedStrings.cpp
    core/CStrings.cpp core/CSysLog.cpp core/CUniqueInstance.cpp core/CVariant.cpp core/string_ext.cpp
    core/CDirectoryDS.cpp core/CMemoryDS.cpp core/CProxyLog.cpp core/CSystemException.cpp
    core/md5.cpp
    net/CBaseMailConnect.cpp net/CBaseSocket.cpp net/CCGI.cpp net/CFTPConnect.cpp
    net/CHttpConnect.cpp net/CHttpParams.cpp net/CImapConnect.cpp net/CMailMessageBody.cpp net/CSmtpConnect.cpp
    net/CSSLContext.cpp net/CSSLSocket.cpp
    net/CTCPServer.cpp net/CTCPSocket.cpp net/CServerConnection.cpp
    net/CUDPSocket.cpp net/CFtpDS.cpp net/CImapDS.cpp
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
