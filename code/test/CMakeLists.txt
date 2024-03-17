IF (GTEST_FLAG)
    FILE(GLOB TEST_SOURCE_FILES tests/*/*.cpp)

    ADD_LIBRARY(sptest ${LIBRARY_TYPE} TestRunner.cpp TestSettings.cpp ${TEST_SOURCE_FILES})
    SET_TARGET_PROPERTIES(sptest PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    TARGET_LINK_LIBRARIES(sptest sputil5 spdb5 spwsdl5 gtest)

    INSTALL(TARGETS sptest RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
    IF (SET_RPATH)
        SET_TARGET_PROPERTIES(sptest PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    ENDIF ()

    ADD_EXECUTABLE(sptk_unit_tests sptk_unit_tests.cpp)
    TARGET_LINK_LIBRARIES(sptk_unit_tests sputil5 spdb5 spwsdl5 sptest gtest)

    INSTALL(TARGETS sptk_unit_tests
            RUNTIME DESTINATION bin COMPONENT unit_tests
            LIBRARY DESTINATION lib COMPONENT unit_tests
            ARCHIVE DESTINATION lib COMPONENT unit_tests)

    IF (SET_RPATH)
        SET_TARGET_PROPERTIES(sptk_unit_tests PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
    ENDIF ()

    IF (BUILD_TEST_WS)
        TARGET_LINK_LIBRARIES(sptk_unit_tests WebServiceUnitTests)
    ENDIF ()

FILE(COPY        ${CMAKE_SOURCE_DIR}/test/settings.txt
     DESTINATION ${CMAKE_BINARY_DIR}/test)
FILE(COPY        ${CMAKE_SOURCE_DIR}/test/data
     DESTINATION ${CMAKE_BINARY_DIR}/test)
FILE(COPY        ${CMAKE_SOURCE_DIR}/test/keys
     DESTINATION ${CMAKE_BINARY_DIR}/test)

ENDIF ()

IF (BUILD_TEST_WS)
    add_subdirectory(wsdl)
ENDIF ()
