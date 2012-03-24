IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

ADD_LIBRARY (sputil5 ${LIBRARY_TYPE}
    core/CBase64.cpp core/CBaseLog.cpp core/CBaseMailConnect.cpp
    core/CBuffer.cpp core/CCGI.cpp core/CDataSource.cpp core/CDateTime.cpp core/CException.cpp
    core/CField.cpp core/CFieldList.cpp core/CFileLog.cpp core/CFTPConnect.cpp
    core/CHttpConnect.cpp core/CHttpParams.cpp core/CImapConnect.cpp core/CIntList.cpp
    core/CMailMessageBody.cpp core/CRegistry.cpp core/CSharedStrings.cpp
    core/CSmtpConnect.cpp core/CSocket.cpp core/CStrings.cpp core/CSysLog.cpp
    core/CUniqueInstance.cpp core/CVariant.cpp core/string_ext.cpp
    core/CDirectoryDS.cpp core/CMemoryDS.cpp core/CFtpDS.cpp core/CImapDS.cpp core/CProxyLog.cpp
    core/CSystemException.cpp
    xml/CXmlAttributes.cpp xml/CXmlDoc.cpp xml/CXmlDocType.cpp xml/CXmlNode.cpp xml/CXmlNodeList.cpp xml/CXmlValue.cpp
    tar/block.cpp tar/CTar.cpp tar/decode.cpp tar/handle.cpp tar/libtar_hash.cpp tar/libtar_list.cpp tar/util.cpp
    threads/CRWLock.cpp threads/CSynchronized.cpp threads/CThread.cpp threads/CThreadPool.cpp
    threads/CSemaphore.cpp threads/CRunable.cpp threads/CWorkerThread.cpp
    )

INSTALL(TARGETS sputil5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
