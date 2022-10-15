# Determines whether or not the compiler supports C++17
macro(check_for_cxx17_compiler _VAR)
    set(${_VAR})
    if ((MSVC AND (MSVC12 OR MSVC13 OR MSVC14 OR MSVC15 OR MSVC16)) OR
    (CMAKE_COMPILER_IS_GNUCXX AND NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8.0) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8.0))
        set(${_VAR} 1)
    endif ()
endmacro()

# Sets the appropriate flag to enable C++17 support
macro(enable_cxx17)
    check_for_cxx17_compiler(IS_17)
    if (IS_17 STREQUAL "1")
        message("Enable Cxx17")
        set(CMAKE_CXX_STANDARD 17)
    endif ()
endmacro()
