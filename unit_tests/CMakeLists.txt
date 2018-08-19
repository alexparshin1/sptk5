IF (GTEST_FLAG)
    ADD_EXECUTABLE(unit_tests unit_tests.cpp)
    TARGET_LINK_LIBRARIES(unit_tests sputil5)
ENDIF()
