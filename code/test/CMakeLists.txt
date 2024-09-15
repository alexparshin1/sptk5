IF (GTEST_FLAG)
    FILE(GLOB TEST_SOURCE_FILES *.cpp tests/*/*.cpp wsdl/*.cpp wsdl/Service/*.cpp)

    ADD_EXECUTABLE(sptk_unit_tests ${TEST_SOURCE_FILES})
    TARGET_LINK_LIBRARIES(sptk_unit_tests sputil5 spdb5 spwsdl5 gtest)

    INSTALL(TARGETS sptk_unit_tests
            RUNTIME DESTINATION bin COMPONENT unit_tests
            LIBRARY DESTINATION lib COMPONENT unit_tests
            ARCHIVE DESTINATION lib COMPONENT unit_tests)

    IF (SET_RPATH)
        SET_TARGET_PROPERTIES(sptk_unit_tests PROPERTIES INSTALL_RPATH "")
    ENDIF ()

    FILE(COPY ${CMAKE_SOURCE_DIR}/test/settings.txt
            DESTINATION ${CMAKE_BINARY_DIR}/test)
    FILE(COPY ${CMAKE_SOURCE_DIR}/test/data
            DESTINATION ${CMAKE_BINARY_DIR}/test)
    FILE(COPY ${CMAKE_SOURCE_DIR}/test/keys
            DESTINATION ${CMAKE_BINARY_DIR}/test)

ENDIF ()
