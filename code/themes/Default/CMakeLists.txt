GET_FILENAME_COMPONENT(THEME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

IF (NOT WIN32)

  SET(THEME_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  ADD_CUSTOM_COMMAND(
    OUTPUT ${THEME}.tar
    DEPENDS ${THEME_DIRECTORY}/*.png theme.ini ${THEME_DIRECTORY}/dialog_icons/*.png ${THEME_DIRECTORY}/icons.12/*.png ${THEME_DIRECTORY}/icons.16/*.png ${THEME_DIRECTORY}/icons.22/*.png
    COMMAND tar
    ARGS --exclude .svn --exclude CMake* --exclude Makefile* --exclude *.tar -cf ${THEME}.tar *
    #COMMAND ../tar2h
    #ARGS Default.tar default_icons > ${SPTK_SOURCE_DIR}/sptk5/gui/default_icons.h
  )

  ADD_CUSTOM_TARGET(
    "${THEME}_Theme" ALL
    DEPENDS ${THEME}.tar
  )

  ADD_DEPENDENCIES("Default_Theme" tar2h)

ENDIF ()

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${THEME}.tar
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sptk5 COMPONENT gui
)
