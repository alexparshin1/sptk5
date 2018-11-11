INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR})
INCLUDE_DIRECTORIES (/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

LINK_DIRECTORIES (${SPTK_BINARY_DIR})
LINK_DIRECTORIES (${FLTK_LIB_DIR})

IF (FLTK_FLAG)

    LINK_LIBRARIES (sputil5 sptk5 spdb5 ${SOCKET_LIBRARY} ${FLTK_LIBRARIES} ${ASPELL_LIBRARIES})
    IF (UNIX)
        LINK_LIBRARIES (pthread rt)
    ENDIF (UNIX)

    SET (GUI_TESTS calendar_test ceditor_test cgroup_test check_buttons combo_test cscroll_test 
                  #db_list_view sptk_test tree_test
                  dialog_test file_dialogs font_combo shaped_window splitter
                  tabs_test themes_test tree_view)

    IF (ASPELL_FLAG)
        SET (GUI_TESTS ${GUI_TESTS} spell_checker)
    ENDIF (ASPELL_FLAG)

    FOREACH (GUI_TEST ${GUI_TESTS})
        IF (WIN32)
            ADD_EXECUTABLE (${GUI_TEST} WIN32 ${GUI_TEST}.cpp)
        ELSE (WIN32)
            ADD_EXECUTABLE (${GUI_TEST} ${GUI_TEST}.cpp)
        ENDIF (WIN32)
    ENDFOREACH (GUI_TEST)

    INSTALL(TARGETS ${GUI_TESTS}
        RUNTIME DESTINATION lib/sptk5/bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

ENDIF (FLTK_FLAG)
