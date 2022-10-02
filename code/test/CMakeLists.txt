IF (GTEST_FLAG)
    ADD_LIBRARY(sptest ${LIBRARY_TYPE} TestRunner.cpp TcpSslServerTests.cpp)
    SET_TARGET_PROPERTIES(sptest PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})
    TARGET_LINK_LIBRARIES(sptest sputil5 spdb5 spwsdl5 gtest)

    INSTALL(TARGETS sptest RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

    ADD_EXECUTABLE(unit_tests unit_tests.cpp)
    TARGET_LINK_LIBRARIES(unit_tests sputil5 spdb5 spwsdl5 sptest gtest)

    IF (BUILD_TEST_WS)
        TARGET_LINK_LIBRARIES(unit_tests WebServiceUnitTests)
    ENDIF ()
ENDIF ()

IF (BUILD_TEST_WS)
    add_subdirectory(wsdl)
ENDIF ()
