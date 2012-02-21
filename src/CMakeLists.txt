INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR})
INCLUDE_DIRECTORIES (/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

IF (WIN32)
  SET (LIBRARY_TYPE STATIC)
ELSE (WIN32)
  #SET (LIBRARY_TYPE STATIC)
  SET (LIBRARY_TYPE SHARED)
ENDIF (WIN32)

ADD_LIBRARY (sputil5 ${LIBRARY_TYPE}
    utils/CBase64.cpp utils/CBaseLog.cpp utils/CBaseMailConnect.cpp 
    utils/CBuffer.cpp utils/CCGI.cpp utils/CDataSource.cpp utils/CDateTime.cpp utils/CException.cpp 
    utils/CField.cpp utils/CFieldList.cpp utils/CFileLog.cpp utils/CFTPConnect.cpp 
    utils/CHttpConnect.cpp utils/CHttpParams.cpp utils/CImapConnect.cpp utils/CIntList.cpp 
    utils/CMailMessageBody.cpp utils/CRegistry.cpp utils/CRWLock.cpp utils/CSharedStrings.cpp 
    utils/CSmtpConnect.cpp utils/CSocket.cpp utils/CStrings.cpp utils/CSysLog.cpp utils/CThread.cpp 
    utils/CUniqueInstance.cpp utils/CVariant.cpp utils/CWaiter.cpp utils/string_ext.cpp 
    utils/CDirectoryDS.cpp utils/CMemoryDS.cpp utils/CFtpDS.cpp utils/CImapDS.cpp utils/CProxyLog.cpp
    xml/CXmlAttributes.cpp xml/CXmlDoc.cpp xml/CXmlDocType.cpp xml/CXmlNode.cpp xml/CXmlNodeList.cpp xml/CXmlValue.cpp
    tar/block.c tar/CTar.cpp tar/decode.c tar/handle.c tar/libtar_hash.c tar/libtar_list.c tar/util.c
    )

SET (SPDB_SOURCES db/CDatabase.cpp db/CDatabaseField.cpp db/CParams.cpp db/CQuery.cpp db/CTransaction.cpp)

SET (ODBC_SOURCES)
IF (ODBC_FLAG)
    SET (ODBC_SOURCES drivers/odbc/CODBC.cpp drivers/odbc/CODBCDatabase.cpp)
ENDIF (ODBC_FLAG)

SET (SQLITE3_SOURCES)
IF (SQLITE3_FLAG)
    SET (SQLITE3_SOURCES drivers/SQLite3/CSQLite3Database.cpp)
ENDIF (SQLITE3_FLAG)

SET (POSTGRESQL_SOURCES)
IF (POSTGRESQL_FLAG)
    SET (POSTGRESQL_SOURCES drivers/PostgreSQL/CPostgreSQLDatabase.cpp)
ENDIF (POSTGRESQL_FLAG)

ADD_LIBRARY (spdb5 ${LIBRARY_TYPE} ${SPDB_SOURCES} ${ODBC_SOURCES} ${SQLITE3_SOURCES} ${POSTGRESQL_SOURCES})

IF (FLTK_FLAG)

  ADD_LIBRARY (sptk5 ${LIBRARY_TYPE} gui/CFontComboBox.cpp gui/CPhoneNumberInput.cpp
    gui/CPngImage.cpp gui/CInput.cpp gui/CFileSaveDialog.cpp gui/CSmallPixmaps.cpp gui/CDateControl.cpp
    gui/CDialog.cpp gui/CHtmlBox.cpp gui/CRadioButtons.cpp gui/CScrollBar.cpp gui/CColumn.cpp gui/CButton.cpp
    gui/CScroll.cpp gui/CTreeControl.cpp gui/CProgressBar.cpp gui/CFileDialog.cpp gui/CFileOpenDialog.cpp
    gui/CLayoutClient.cpp gui/CTreeView.cpp gui/CGroup.cpp gui/CDropDownBox.cpp gui/CCheckButtons.cpp
    gui/CFont.cpp gui/CDateIntervalInput.cpp gui/CLayoutManager.cpp gui/CDateTimeInput.cpp
    gui/CWindowShape.cpp gui/CSplitter.cpp gui/CIntegerInput.cpp gui/CToolBar.cpp gui/CEditor.cpp
    gui/CPackedStrings.cpp gui/CMemoInput.cpp gui/CControl.cpp gui/CListViewRows.cpp gui/CWindow.cpp
    gui/CPasswordInput.cpp gui/CBox.cpp gui/CButtonGroup.cpp gui/CComboBox.cpp gui/CFloatInput.cpp
    gui/CPopupWindow.cpp gui/CListView.cpp gui/CTabs.cpp gui/CDataControl.cpp gui/CDBListView.cpp
    gui/CListViewSelection.cpp gui/CControlList.cpp gui/CMessageDialog.cpp gui/CEditorSpellChecker.cpp
    gui/CDirOpenDialog.cpp gui/CCalendar.cpp
    themes/CColorSchema.cpp themes/CGtkThemeLoader.cpp themes/CThemes.cpp themes/CThemesGtk.cpp themes/CIcon.cpp
    themes/CFrame.cpp themes/CTabImage.cpp themes/ThemeUtils.cpp themes/CThemeImageCollection.cpp
    themes/CThemeColorCollection.cpp themes/CThemeScrollBar.cpp)

  ADD_DEPENDENCIES (sptk5 "Default Theme")

  IF (WIN32)
    SET_TARGET_PROPERTIES (sptk5 PROPERTIES LINK_FLAGS "-mwindows -mno-cygwin")
  ENDIF (WIN32)

  TARGET_LINK_LIBRARIES (sptk5 spdb5 sputil5)
  INSTALL(TARGETS sptk5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

ENDIF (FLTK_FLAG)

IF (WIN32)
  SET_TARGET_PROPERTIES (sputil5 spdb5 PROPERTIES LINK_FLAGS "-mwindows -mno-cygwin")
ENDIF (WIN32)

IF (TLS_FLAG)
  TARGET_LINK_LIBRARIES (sputil5 ${TLS_LIBRARY})
ENDIF (TLS_FLAG)

TARGET_LINK_LIBRARIES (spdb5 sputil5)

INSTALL(TARGETS sputil5 spdb5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
