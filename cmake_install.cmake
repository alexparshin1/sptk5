# Install script for directory: /home/alexeyp/workspace/sptk5

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

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5" TYPE FILE FILES
    "/home/alexeyp/workspace/sptk5/sptk5/CWaiter.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CLogFile.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CDirectoryDS.h"
    "/home/alexeyp/workspace/sptk5/sptk5/sptk-config.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CBaseMailConnect.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CFrame.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CMemoryDS.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CSysLog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CGuard.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CStrings.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CCGI.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CDataSource.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CBase64.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CProxyLog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CHttpConnect.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CBuffer.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CImapDS.h"
    "/home/alexeyp/workspace/sptk5/sptk5/sptk.h"
    "/home/alexeyp/workspace/sptk5/sptk5/DebugMacros.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CMailMessageBody.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CVariant.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CStringStack.h"
    "/home/alexeyp/workspace/sptk5/sptk5/string_ext.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CFileLog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CUniqueInstance.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CTar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CFtpDS.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CHttpParams.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CSocket.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CIntList.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CPackedStrings.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CSharedStrings.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CSmallPixmapIDs.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CBaseLog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CField.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CException.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CFieldList.h"
    "/home/alexeyp/workspace/sptk5/sptk5/filedefs.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CDateTime.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CSmtpConnect.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CRWLock.h"
    "/home/alexeyp/workspace/sptk5/sptk5/istring.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CRegistry.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CThread.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CFTPConnect.h"
    "/home/alexeyp/workspace/sptk5/sptk5/CImapConnect.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5/xml" TYPE FILE FILES
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlNodeList.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlAttributes.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlValue.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlNode.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlEntities.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlElement.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXml.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlDocType.h"
    "/home/alexeyp/workspace/sptk5/sptk5/xml/CXmlDoc.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5/db" TYPE FILE FILES
    "/home/alexeyp/workspace/sptk5/sptk5/db/CQuery.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CPostgreSQLDatabase.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CDatabase.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CQueryGuard.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CODBCDatabase.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CDatabaseField.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CSQLite3Database.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CParams.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CTransaction.h"
    "/home/alexeyp/workspace/sptk5/sptk5/db/CODBC.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5/gui" TYPE FILE FILES
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFont.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CEvent.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CColumn.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CToolBar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CButton.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CProgressBar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CThemeImageCollection.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDateControl.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CShapePoint.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CScrollBar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFrame.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CPasswordInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CPngImage.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFloatInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CGroup.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CCalendar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CColorSchema.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFileDialog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CMenuBar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDialog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDataControl.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CListViewSelection.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CTreeView.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CIntegerInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CCheckButtons.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CThemes.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CPhoneNumberInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CMessageDialog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CWindowShape.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CControlList.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CControl.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CMemoInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CThemeImageState.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CButtonGroup.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CTabs.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDropDownBox.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CPopupWindow.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFileSaveDialog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFontComboBox.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDirOpenDialog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CFileOpenDialog.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CListViewRows.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CRect.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CSplitter.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CThemeScrollBar.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CEditor.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CTreeControl.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CScroll.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CBox.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CWindow.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/default_icons.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CListView.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CHtmlBox.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CEditorSpellChecker.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CLayoutManager.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDBListView.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CLayoutClient.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDateIntervalInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CTabImage.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CDateTimeInput.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CRadioButtons.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CThemeColorCollection.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CIcon.h"
    "/home/alexeyp/workspace/sptk5/sptk5/gui/CComboBox.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sptk5" TYPE FILE FILES
    "/home/alexeyp/workspace/sptk5/sptk5/cutils"
    "/home/alexeyp/workspace/sptk5/sptk5/cxml"
    "/home/alexeyp/workspace/sptk5/sptk5/cdatabase"
    "/home/alexeyp/workspace/sptk5/sptk5/cgui"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/home/alexeyp/workspace/sptk5/dbtools/sql2cpp.pl")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/sptk/doc" TYPE FILE FILES "/home/alexeyp/workspace/sptk5/dbtools/sql2cpp.conf")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/alexeyp/workspace/sptk5/themes/cmake_install.cmake")
  INCLUDE("/home/alexeyp/workspace/sptk5/src/cmake_install.cmake")
  INCLUDE("/home/alexeyp/workspace/sptk5/examples/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/alexeyp/workspace/sptk5/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/alexeyp/workspace/sptk5/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
