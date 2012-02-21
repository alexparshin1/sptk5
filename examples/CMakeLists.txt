IF (NO_EXAMPLES)
  MESSAGE("Usage examples:     don't compile")
ELSE (NO_EXAMPLES)
  MESSAGE("Usage examples:     compile")

  ADD_SUBDIRECTORY (xml)
  ADD_SUBDIRECTORY (database)

  INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
  INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR})
  INCLUDE_DIRECTORIES (/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

  LINK_DIRECTORIES (${SPTK_BINARY_DIR})
  LINK_DIRECTORIES (${FLTK_LIB_DIR})

  LINK_LIBRARIES (sputil5 ${ODBC_LIBRARY} ${SQLITE3_LIBRARY} ${POSTGRESQL_LIBRARY} ${SOCKET_LIBRARY} pthread)

  SET (CONSOLE_TESTS datetime client_test server_test exceptions logfile_test thread_onexit
            thread_rwlock thread_test syslog_test unique_instance
            #xml/xml_test2
            registry
  )

  FOREACH (CONSOLE_TEST ${CONSOLE_TESTS})

    ADD_EXECUTABLE (${CONSOLE_TEST} ${CONSOLE_TEST}.cpp)

    TARGET_LINK_LIBRARIES (${CONSOLE_TEST} sputil5)

    IF (WIN32)
      SET_TARGET_PROPERTIES (${CONSOLE_TEST} PROPERTIES LINK_FLAGS "-mconsole -mno-cygwin")
    ENDIF (WIN32)

  ENDFOREACH (CONSOLE_TEST)

  INSTALL(TARGETS ${CONSOLE_TESTS}
    RUNTIME DESTINATION share/sptk/bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  ################## GUI tests ####################################
  IF (FLTK_FLAG)
    LINK_LIBRARIES (sputil5 sptk5 spdb5 ${ODBC_LIBRARY} ${SQLITE3_LIBRARY} ${POSTGRESQL_LIBRARY} ${SOCKET_LIBRARY} ${FLTK_LIBRARIES} ${ASPELL_LIBRARIES} pthread)
    #SET (GUI_LIBRARIES "-lsputil5 -lsptk5 -lspdb5 ${FLTK_LIBRARIES} ${ASPELL_LIBRARIES}")

    SET (GUI_TESTS calendar_test ceditor_test cgroup_test check_buttons combo_test cscroll_test db_list_view
                  dialog_test font_combo shaped_window spell_checker splitter sptk_test 
                  tabs_test themes_test tree_test tree_view)

    FOREACH (GUI_TEST ${GUI_TESTS})
      ADD_EXECUTABLE (${GUI_TEST} ${GUI_TEST}.cpp)
      IF (WIN32)
        SET_TARGET_PROPERTIES (${GUI_TEST} PROPERTIES LINK_FLAGS "-mwindows -mno-cygwin")
      ENDIF (WIN32)
      #TARGET_LINK_LIBRARIES (${GUI_TEST} ${GUI_LIBRARIES})
    ENDFOREACH (GUI_TEST)

    INSTALL(TARGETS ${GUI_TESTS}
      RUNTIME DESTINATION share/sptk/bin
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
    )

    ADD_SUBDIRECTORY (datasources)
    ADD_SUBDIRECTORY (dialogs)
    ADD_SUBDIRECTORY (net)

  ENDIF (FLTK_FLAG)

ENDIF (NO_EXAMPLES)
