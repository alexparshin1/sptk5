INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR})
INCLUDE_DIRECTORIES (/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

LINK_DIRECTORIES (${SPTK_BINARY_DIR})
LINK_DIRECTORIES (${FLTK_LIB_DIR})

IF (FLTK_FLAG)

    LINK_LIBRARIES (sputil5 sptk5 spdb5 ${ODBC_LIBRARY} ${SQLITE3_LIBRARY} ${POSTGRESQL_LIBRARY} ${SOCKET_LIBRARY} ${FLTK_LIBRARIES} ${ASPELL_LIBRARIES} pthread)

    SET (GUI_TESTS calendar_test ceditor_test cgroup_test check_buttons combo_test cscroll_test db_list_view
                  dialog_test file_dialogs font_combo shaped_window spell_checker splitter sptk_test
                  tabs_test themes_test tree_test tree_view)

    FOREACH (GUI_TEST ${GUI_TESTS})
        ADD_EXECUTABLE (${GUI_TEST} ${GUI_TEST}.cpp)
        IF (WIN32)
            SET_TARGET_PROPERTIES (${GUI_TEST} PROPERTIES LINK_FLAGS "-mwindows -mno-cygwin")
        ENDIF (WIN32)
    ENDFOREACH (GUI_TEST)

    INSTALL(TARGETS ${GUI_TESTS}
        RUNTIME DESTINATION share/sptk/bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

ENDIF (FLTK_FLAG)
