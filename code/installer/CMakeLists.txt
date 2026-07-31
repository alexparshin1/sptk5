INCLUDE_DIRECTORIES(${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIR})
INCLUDE_DIRECTORIES(/usr/local/include /usr/include/pgsql /usr/include/pgsql/libpq)

LINK_DIRECTORIES(${SPTK_BINARY_DIR})
LINK_DIRECTORIES(${FLTK_LIB_DIR})

IF (FLTK_FLAG)

    LINK_LIBRARIES(sputil5 sptk5 spdb5 ${SOCKET_LIBRARY} ${FLTK_LIBRARIES} ${ASPELL_LIBRARIES})
    IF (UNIX)
        LINK_LIBRARIES(pthread rt)
    ENDIF (UNIX)

    FILE(GLOB INSTALLER_SOURCE_FILES *.h *.cpp)

    IF (WIN32)
        ADD_EXECUTABLE(spinst WIN32 ${INSTALLER_SOURCE_FILES})
    ELSE (WIN32)
        ADD_EXECUTABLE(spinst ${INSTALLER_SOURCE_FILES})
    ENDIF (WIN32)

    IF (SET_RPATH)
        SET_TARGET_PROPERTIES(spinst PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
    ENDIF ()

    INSTALL(TARGETS spinst
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )

    MESSAGE("COPY:    ${CMAKE_CURRENT_LIST_DIR}/images")
    MESSAGE("COPY TO: ${CMAKE_CURRENT_BINARY_DIR}/images")
    add_custom_target(copy_installer_images
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_LIST_DIR}/images ${CMAKE_CURRENT_BINARY_DIR}/images
    )
    add_custom_target(copy_installer_configs
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_LIST_DIR}/config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json
    )
    add_dependencies(spinst copy_installer_images copy_installer_configs)

ENDIF (FLTK_FLAG)
