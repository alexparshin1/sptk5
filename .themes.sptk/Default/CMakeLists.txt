EXEC_PROGRAM(basename ARGS ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE THEME) 

ADD_CUSTOM_COMMAND( 
    OUTPUT ${THEME}.tar
    DEPENDS *.png theme.ini dialog_icons/*.png icons.12/*.png icons.16/*.png icons.22/*.png
    COMMAND tar 
    ARGS cf ${THEME}.tar * --exclude .svn --exclude CMake* --exclude Makefile* --exclude *.tar
    COMMAND ../tar2h
    ARGS Default.tar default_icons > ../../sptk3/gui/default_icons.h
)

ADD_CUSTOM_TARGET(
    "${THEME} Theme" ALL
    DEPENDS ${THEME}.tar
)

ADD_DEPENDENCIES ("Default Theme" tar2h)

INSTALL_FILES(/share/sptk .tar ${THEME}.tar)
