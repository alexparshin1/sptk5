GET_FILENAME_COMPONENT(THEME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

IF (NOT WIN32)

    ADD_CUSTOM_COMMAND( 
        OUTPUT ${THEME}.tar
        DEPENDS *.png theme.ini dialog_icons/*.png icons.12/*.png icons.16/*.png icons.22/*.png
        COMMAND tar
        ARGS --exclude .svn --exclude CMake* --exclude Makefile* --exclude *.tar -cf ${THEME}.tar *
    )

    ADD_CUSTOM_TARGET(
        "${THEME}_Theme" ALL
        DEPENDS ${THEME}.tar
    )

ENDIF()

INSTALL_FILES(/share/sptk5 .tar ${THEME}.tar)
