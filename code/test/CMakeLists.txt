IF (GTEST_FLAG)
    FILE(GLOB TEST_SOURCE_FILES *.cpp tests/*.cpp tests/*/*.cpp wsdl/*.cpp wsdl/Service/*.cpp)

    ADD_EXECUTABLE(sptk_unit_tests ${TEST_SOURCE_FILES})
    SET_SPTK_TARGET_PROPERTIES(sptk_unit_tests)

    TARGET_LINK_LIBRARIES(sptk_unit_tests spdb5_static spwsdl5_static sputil5_static gtest)

    # On Windows with MSVC, add googletest as a Visual Studio NuGet package.
    #if (WIN32 AND MSVC)
    #    set(gtest_nuget "Google.Test;1.8.3")
    #    set_property(TARGET sptk_unit_tests PROPERTY VS_PACKAGE_REFERENCES "${gtest_nuget}")
    #endif()

    INSTALL(TARGETS sptk_unit_tests
            RUNTIME DESTINATION bin COMPONENT unit_tests
            LIBRARY DESTINATION lib COMPONENT unit_tests
            ARCHIVE DESTINATION lib COMPONENT unit_tests)

    IF (WIN32)
        INSTALL(DIRECTORY test_data DESTINATION ${DATADIR}/SPTK COMPONENT unit_tests)
        INSTALL(DIRECTORY test_ssl_keys DESTINATION ${DATADIR}/SPTK COMPONENT unit_tests)
        INSTALL(FILES settings.txt DESTINATION ${DATADIR}/SPTK COMPONENT unit_tests)
    ELSE ()
        INSTALL(DIRECTORY test_data DESTINATION /usr/local/share/sptk5 COMPONENT unit_tests)
        INSTALL(DIRECTORY test_ssl_keys DESTINATION /usr/local/share/sptk5 COMPONENT unit_tests)
        INSTALL(FILES settings.txt DESTINATION /usr/local/share/sptk5 COMPONENT unit_tests)
    ENDIF ()

ENDIF ()
