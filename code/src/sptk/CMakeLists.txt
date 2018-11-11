IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

IF (FLTK_FLAG)

  INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR})

  IF (WIN32)
    INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR}/png ${FLTK_INCLUDE_DIR}/zlib)
  ENDIF (WIN32)

  SET (SPTK_SOURCES gui/CFontComboBox.cpp gui/CPhoneNumberInput.cpp
    gui/CPngImage.cpp gui/CInput.cpp gui/CFileSaveDialog.cpp gui/CSmallPixmaps.cpp gui/CDateControl.cpp
    gui/CDialog.cpp gui/CHtmlBox.cpp gui/CRadioButtons.cpp gui/CScrollBar.cpp gui/CColumn.cpp gui/CButton.cpp
    gui/CScroll.cpp gui/CTreeControl.cpp gui/CProgressBar.cpp gui/CFileDialog.cpp gui/CFileOpenDialog.cpp
    gui/CLayoutClient.cpp gui/CTreeView.cpp gui/CGroup.cpp gui/CDropDownBox.cpp gui/CCheckButtons.cpp
    gui/CFont.cpp gui/CDateIntervalInput.cpp gui/CLayoutManager.cpp gui/CDateTimeInput.cpp
    gui/CWindowShape.cpp gui/CSplitter.cpp gui/CIntegerInput.cpp gui/CToolBar.cpp gui/CEditor.cpp
    gui/CPackedStrings.cpp gui/CMemoInput.cpp gui/CControl.cpp gui/CListViewRows.cpp gui/CWindow.cpp
    gui/CPasswordInput.cpp gui/CBox.cpp gui/CButtonGroup.cpp gui/CComboBox.cpp gui/CFloatInput.cpp
    gui/CPopupWindow.cpp gui/CListView.cpp gui/CTabs.cpp gui/CDataControl.cpp gui/CDBListView.cpp
    gui/CListViewSelection.cpp gui/CControlList.cpp gui/CMessageDialog.cpp
    gui/CDirOpenDialog.cpp gui/CCalendar.cpp
    themes/CColorSchema.cpp themes/CGtkThemeLoader.cpp themes/CThemes.cpp themes/CThemesGtk.cpp themes/CIcon.cpp
    themes/CFrame.cpp themes/CTabImage.cpp themes/ThemeUtils.cpp themes/CThemeImageCollection.cpp
    themes/CThemeColorCollection.cpp themes/CThemeScrollBar.cpp)

  IF (ASPELL_FLAG)
    SET (SPTK_SOURCES ${SPTK_SOURCES} gui/CEditorSpellChecker.cpp)
  ENDIF (ASPELL_FLAG)

  ADD_LIBRARY (sptk5 ${LIBRARY_TYPE} ${SPTK_SOURCES})
  SET_TARGET_PROPERTIES(sptk5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

  ADD_DEPENDENCIES (sptk5 "Default_Theme")

  TARGET_LINK_LIBRARIES (sptk5 spdb5 sputil5 png)
  INSTALL(TARGETS sptk5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

ENDIF (FLTK_FLAG)
