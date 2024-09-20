IF (GTEST_FLAG)
    FILE(GLOB TEST_SOURCE_FILES *.cpp tests/*.cpp tests/*/*.cpp wsdl/*.cpp wsdl/Service/*.cpp)

    ADD_EXECUTABLE(sptk_unit_tests ${TEST_SOURCE_FILES})

    TARGET_LINK_LIBRARIES(sptk_unit_tests sputil5 spdb5 spwsdl5 gtest)

    INSTALL(TARGETS sptk_unit_tests
            RUNTIME DESTINATION bin COMPONENT unit_tests
            LIBRARY DESTINATION lib COMPONENT unit_tests
            ARCHIVE DESTINATION lib COMPONENT unit_tests)

    INSTALL(DIRECTORY test_data DESTINATION /usr/local/share/sptk5 COMPONENT unit_tests)
    INSTALL(DIRECTORY test_ssl_keys DESTINATION /usr/local/share/sptk5 COMPONENT unit_tests)
    INSTALL(FILES settings.txt DESTINATION /usr/local/share/sptk5 COMPONENT unit_tests)

    IF (SET_RPATH)
        SET_TARGET_PROPERTIES(sptk_unit_tests PROPERTIES INSTALL_RPATH "")
    ENDIF ()

ENDIF ()
