# Install script for directory: /cygdrive/c/workspace/sptk5

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5" TYPE FILE FILES
    "/cygdrive/c/workspace/sptk5/sptk5/CBase64.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CBaseLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CBaseMailConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CBuffer.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CCGI.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CDataSource.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CDateTime.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CDirectoryDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CException.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CField.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFieldList.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFileLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFrame.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFTPConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFtpDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CHttpConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CHttpParams.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CImapConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CImapDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CIntList.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CLogFile.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CMailMessageBody.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CMemoryDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CPackedStrings.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CProxyLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CRegistry.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSharedStrings.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSmallPixmapIDs.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSmtpConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSocket.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CStrings.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CStringStack.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSysLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSystemException.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CTar.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CUniqueInstance.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CVariant.h"
    "/cygdrive/c/workspace/sptk5/sptk5/DebugMacros.h"
    "/cygdrive/c/workspace/sptk5/sptk5/filedefs.h"
    "/cygdrive/c/workspace/sptk5/sptk5/istring.h"
    "/cygdrive/c/workspace/sptk5/sptk5/sptk-config.h"
    "/cygdrive/c/workspace/sptk5/sptk5/sptk.h"
    "/cygdrive/c/workspace/sptk5/sptk5/string_ext.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5/xml" TYPE FILE FILES
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXml.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlAttributes.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlDoc.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlDocType.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlElement.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlEntities.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlNode.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlNodeList.h"
    "/cygdrive/c/workspace/sptk5/sptk5/xml/CXmlValue.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5/db" TYPE FILE FILES
    "/cygdrive/c/workspace/sptk5/sptk5/db/CDatabaseDriver.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CDatabaseField.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CODBC.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CODBCDatabase.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CParams.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CPostgreSQLDatabase.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CQuery.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CQueryGuard.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CSQLite3Database.h"
    "/cygdrive/c/workspace/sptk5/sptk5/db/CTransaction.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5" TYPE FILE FILES
    "/cygdrive/c/workspace/sptk5/sptk5/CBase64.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CBaseLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CBaseMailConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CBuffer.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CCGI.h"
    "/cygdrive/c/workspace/sptk5/sptk5/cdatabase"
    "/cygdrive/c/workspace/sptk5/sptk5/CDataSource.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CDateTime.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CDirectoryDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CException.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CField.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFieldList.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFileLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFrame.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFTPConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CFtpDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/cgui"
    "/cygdrive/c/workspace/sptk5/sptk5/CHttpConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CHttpParams.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CImapConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CImapDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CIntList.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CLogFile.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CMailMessageBody.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CMemoryDS.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CPackedStrings.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CProxyLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CRegistry.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSharedStrings.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSmallPixmapIDs.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSmtpConnect.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSocket.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CStrings.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CStringStack.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSysLog.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CSystemException.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CTar.h"
    "/cygdrive/c/workspace/sptk5/sptk5/CUniqueInstance.h"
    "/cygdrive/c/workspace/sptk5/sptk5/cutils"
    "/cygdrive/c/workspace/sptk5/sptk5/CVariant.h"
    "/cygdrive/c/workspace/sptk5/sptk5/cxml"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/cygdrive/c/workspace/sptk5/src/cmake_install.cmake")
  INCLUDE("/cygdrive/c/workspace/sptk5/examples/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/cygdrive/c/workspace/sptk5/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/cygdrive/c/workspace/sptk5/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
