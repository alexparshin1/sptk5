CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# The name of the project is "SPTK".  CMakeLists files in this project can
# refer to the root source directory of the project as ${SPTK_SOURCE_DIR} and
# to the root binary directory of the project as ${SPTK_BINARY_DIR}.
PROJECT(SPTK)

#cmake_policy(SET CMP0046 OLD)

#SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)

SET(VERSION "5.4.1")
SET(VERSION_MAJOR "5")
SET(VERSION_MINOR "4")
SET(VERSION_PATCH "1")
SET(SOVERSION "5")

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Place compiled libraries and examples into ./bin/
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

SET(TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
SET(CMAKE_INSTALL_RPATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

IF (UNIX)
    OPTION(USE_NEW_ABI "Use new GCC ABI" OFF)
ENDIF ()

OPTION(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." ON)
OPTION(USE_EPOLL "Build Asynchronous sockets (epoll) support" ON)
OPTION(USE_FLTK "Build GUI library" ON)
OPTION(USE_MYSQL "Build MySQL support library" ON)
OPTION(USE_ODBC "Build ODBC support library" ON)
OPTION(USE_OPENSSL "Build OpenSSL support library" ON)
OPTION(USE_ORACLE "Build Oracle support library" ON)
OPTION(USE_POSTGRESQL "Build PostgreSQL support library" ON)
OPTION(USE_SQLITE3 "Build SQLite3 support library" ON)
OPTION(USE_GTEST "Build Google Test unit tests" ON)

OPTION(USE_PCRE "Use PCRE library" ON)
OPTION(USE_BROTLI "Use Brotli library" ON)

OPTION(BUILD_EXAMPLES "Build usage examples" ON)
OPTION(BUILD_UTILS "Build utitlity programs" ON)
OPTION(BUILD_PACKAGE "Build OS package" ON)

IF (NOT WIN32)
    OPTION(BUILD_TEST_WS "Build Test Web Service" ON)
ENDIF ()

INCLUDE(${CMAKE_ROOT}/Modules/FindASPELL.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

INCLUDE(cmake/OSType.cmake)

INCLUDE(CheckTypeSize)
check_type_size("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)
SET(BITNESS 32)
IF (SIZEOF_VOID_P EQUAL 8)
    SET(BITNESS 64)
ENDIF ()

MESSAGE("Build type:         ${CMAKE_BUILD_TYPE} ${BITNESS} bit")
MESSAGE("Compiler:           ${CMAKE_CXX_COMPILER_ID}")
IF (WIN32)
    ADD_DEFINITIONS(
        -DHAVE_CONFIG_H
        -DWIN32
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        -D_CRT_SECURE_NO_WARNINGS
        -DGTEST_LANG_CXX11=1
    )
    ADD_COMPILE_OPTIONS(/wd4275 /std:c++20)
    LINK_LIBRARIES(wsock32 ole32 uuid comctl32 ws2_32 winhttp)
ELSE ()
    IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        ADD_DEFINITIONS(
            -fPIC
            -Wall -pedantic
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-cast-align
            -Wno-exit-time-destructors
            -Wno-float-equal
            -Wno-format-nonliteral
            -Wno-global-constructors
            -Wno-implicit-fallthrough
            -Wno-long-long
            -Wno-missing-prototypes
            -Wno-missing-noreturn
            -Wno-padded
            -Wno-shadow
            -Wno-switch-enum
            -Wno-unused-parameter
            #-fprofile-arcs -ftest-coverage
        )
    ELSE ()
        ADD_DEFINITIONS(
            -fPIC
            -Wall -pedantic
            -Wno-long-long
            #--coverage
        )
    ENDIF ()

    IF (USE_NEW_ABI)
        SET(NEW_ABI_FLAG "1")
        SET(NEW_ABI_DEFINE "#define USE_NEW_ABI")
        MESSAGE("GCC ABI:            1")
        ADD_DEFINITIONS(-D_GLIBCXX_USE_CXX11_ABI=1)
    ELSE ()
        # For Oracle client libraries, we need compatibility with old ABI
        SET(NEW_ABI_FLAG "0")
        SET(NEW_ABI_DEFINE "// New C++ ABI not used")
        MESSAGE("GCC ABI:            0")
        ADD_DEFINITIONS(-D_GLIBCXX_USE_CXX11_ABI=0)
    ENDIF ()

    IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
        ADD_DEFINITIONS(-O2)
    ELSE ()
        ADD_DEFINITIONS(-O3)
    ENDIF ()

    IF (UNIX)
        LINK_LIBRARIES(pthread rt dl stdc++fs)
        #LINK_LIBRARIES(gcov)
    ELSE ()
        LINK_LIBRARIES(pthread rt)
    ENDIF ()
ENDIF ()

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DSPTK_LOCK_DEBUG)
ENDIF ()


IF (WIN32)
    SET(WINDIR "$ENV{ProgramW6432}")
    IF (NOT WINDIR)
        SET(WINDIR "$ENV{ProgramFiles}")
    ENDIF ()
    SET(CMAKE_INSTALL_PREFIX "${WINDIR}/SPTK" CACHE PATH "Cmake prefix" FORCE)
    SET(LIBRARY_TYPE SHARED)
    IF (LIBRARY_TYPE STREQUAL "SHARED")
        ADD_DEFINITIONS(-DSP_DLL)
    ENDIF (LIBRARY_TYPE STREQUAL "SHARED")
ELSE (WIN32)
    #SET (LIBRARY_TYPE STATIC)
    SET(LIBRARY_TYPE SHARED)
ENDIF (WIN32)
MESSAGE("Library type:       " ${LIBRARY_TYPE})

INCLUDE_DIRECTORIES(".")
INCLUDE_DIRECTORIES(${SPTK_SOURCE_DIR})

SET(ODBC_FLAG "0")
IF (USE_ODBC)
    IF (WIN32)
        SET(ODBC_FLAG "1")
        SET(ODBC_LIBRARY "odbc32")
    ELSE (WIN32)
        FIND_LIBRARY(ODBC_LIBRARY NAMES odbc PATHS /usr/lib /usr/lib/* /usr/local/lib)
    ENDIF (WIN32)

    IF (ODBC_LIBRARY)
        SET(ODBC_FLAG "1")
    ENDIF (ODBC_LIBRARY)
ENDIF (USE_ODBC)

IF (ODBC_FLAG)
    MESSAGE("ODBC support:       compile")
    SET(ODBC_DEFINE "#define HAVE_ODBC")
ELSE (ODBC_FLAG)
    MESSAGE("ODBC support:       don't compile")
    SET(ODBC_LIBRARY "")
    SET(ODBC_DEFINE "// ODBC is not used")
ENDIF (ODBC_FLAG)


FIND_LIBRARY(SOCKET_LIBRARY NAMES socket PATHS /usr/lib)
IF (NOT SOCKET_LIBRARY)
    SET(SOCKET_LIBRARY "")
ELSE (NOT SOCKET_LIBRARY)
    SET(SOCKET_LIBRARY "-lsocket -lrt -lnsl")
ENDIF (NOT SOCKET_LIBRARY)


SET(POSTGRESQL_FLAG "0")
SET(POSTGRESQL_DEFINE "// PostgreSQL is not used")
IF (USE_POSTGRESQL)
    FIND_PACKAGE(PostgreSQL QUIET)
    IF (PostgreSQL_FOUND)
        MESSAGE("PostgreSQL support: compile")
        SET(POSTGRESQL_FLAG "1")
        SET(POSTGRESQL_DEFINE "#define HAVE_POSTGRESQL")
    ELSE (PostgreSQL_FOUND)
        MESSAGE("PostgreSQL support: don't compile")
    ENDIF (PostgreSQL_FOUND)
ENDIF (USE_POSTGRESQL)


SET(EPOLL_FLAG "0")
SET(EPOLL_DEFINE "// EPoll is not used")
IF (USE_EPOLL)
    FIND_PACKAGE(EPoll QUIET)
    IF (EPOLL_FOUND)
        MESSAGE("EPoll support:      compile")
        SET(EPOLL_FLAG "1")
        INCLUDE_DIRECTORIES(${EPOLL_INCLUDE_DIR})
        LINK_LIBRARIES(${EPOLL_LIBRARY})
        SET(EPOLL_DEFINE "#define HAVE_EPOLL")
    ELSE (EPOLL_FOUND)
        MESSAGE("EPoll support:      don't compile")
    ENDIF (EPOLL_FOUND)
ENDIF (USE_EPOLL)


SET(ORACLE_FLAG "0")
SET(ORACLE_DEFINE "// Oracle SQL is not used")
IF (USE_ORACLE)
    FIND_PACKAGE(Oracle QUIET)
    IF (Oracle_FOUND)
        MESSAGE("Oracle support:     compile")
        SET(ORACLE_FLAG "1")
        SET(ORACLE_DEFINE "#define HAVE_ORACLE")
    ELSE (Oracle_FOUND)
        MESSAGE("Oracle support:     don't compile")
    ENDIF (Oracle_FOUND)
ENDIF (USE_ORACLE)


SET(MYSQL_FLAG "0")
SET(MYSQL_DEFINE "// MySQL is not used")
SET(MARIADB_DEFINE "// MariaDB is not used")
SET(MYSQL_HAS_MY_BOOL_DEFINE "// MySQL doesn't define my_bool")
IF (USE_MYSQL)
    FIND_PACKAGE(MySQL QUIET)
    IF (MySQL_FOUND)
        SET(MYSQL_DEFINE "#define HAVE_MYSQL")
        IF (MARIADB_FLAG)
            MESSAGE("MariaDB support:    compile")
            SET(MARIADB_DEFINE "#define HAVE_MARIADB")
        ELSE ()
            MESSAGE("MySQL support:      compile")
        ENDIF ()
        SET(MYSQL_FLAG "1")
        IF (MySQL_has_my_bool)
            SET(MYSQL_HAS_MY_BOOL_DEFINE "#define MYSQL_HAS_MYBOOL")
        ENDIF ()
    ELSE ()
        MESSAGE("MySQL support:      don't compile")
    ENDIF ()
ENDIF ()


SET(SQLITE3_FLAG "0")
IF (USE_SQLITE3)
    FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 PATHS /usr/lib /usr/local/lib)
    IF (NOT SQLITE3_LIBRARY AND WIN32)
        FIND_FILE(SQLITE3_LIBRARY sqlite3.dll PATHS /usr/lib /usr/local/lib)
    ENDIF (NOT SQLITE3_LIBRARY AND WIN32)
    IF (SQLITE3_LIBRARY)
        SET(SQLITE3_FLAG "1")
    ELSE (SQLITE3_LIBRARY)
        SET(SQLITE3_LIBRARY "")
    ENDIF (SQLITE3_LIBRARY)
ENDIF (USE_SQLITE3)

IF (SQLITE3_FLAG)
    MESSAGE("SQLITE3 support:    compile")
    SET(SQLITE3_DEFINE "#define HAVE_SQLITE3")
ELSE ()
    MESSAGE("SQLITE3 support:    don't compile")
    SET(SQLITE3_DEFINE "// SQLite3 is not used")
ENDIF ()


# Detect FLTK and Aspell
SET(FLTK_FLAG "0")
SET(ASPELL_FLAG "0")
IF (USE_FLTK)
    FIND_PACKAGE(FLTK13 QUIET)
    IF (FLTK_FOUND)
        SET(FLTK_FLAG "1")
        #ADD_DEFINITIONS (${FLTK_CXX_FLAGS})
    ENDIF (FLTK_FOUND)
    IF (FLTK_FOUND)
        FIND_PACKAGE(ASPELL QUIET)
        IF (ASPELL_FOUND)
            SET(ASPELL_FLAG "1")
        ELSE (ASPELL_FOUND)
            SET(ASPELL_LIBRARIES "")
        ENDIF (ASPELL_FOUND)

        IF (NOT WIN32)
            FIND_PACKAGE(PNG QUIET)
            IF (NOT PNG_FOUND)
                MESSAGE(FATAL_ERROR "Please istall libpng (devel)")
            ENDIF ()

            FIND_PACKAGE(X11 QUIET)
            IF (NOT X11_Xshape_FOUND)
                MESSAGE(FATAL_ERROR "Please istall libXext (devel)")
            ENDIF ()
        ENDIF ()
    ENDIF (FLTK_FOUND)
ENDIF (USE_FLTK)

SET(ASPELL_DEFINE "// ASpell is not used")
SET(FLTK_DEFINE "// FLTK is not used")
IF (FLTK_FOUND)
    MESSAGE("FLTK support:       compile")
    SET(FLTK_DEFINE "#define HAVE_FLTK")

    IF (ASPELL_FLAG)
        MESSAGE("ASPELL support:     compile")
        SET(ASPELL_DEFINE "#define HAVE_ASPELL")
    ELSE (ASPELL_FLAG)
        MESSAGE("ASPELL support:     don't compile")
    ENDIF (ASPELL_FLAG)

ELSE ()
    MESSAGE("FLTK support:       don't compile")
ENDIF ()


IF (WIN32)
    # Windows message resource compiler (included with Windows SDK)
    SET(ProgramFiles "$ENV{ProgramFiles}")
    SET(ProgramFilesX86 "${ProgramFiles} (x86)")
    FIND_PROGRAM(MC_COMPILER mc PATHS
        "${ProgramFiles}/Windows Kits/*/bin/x64"
        "${ProgramFilesX86}/Windows Kits/*/bin/x64"
        "${ProgramFiles}/Microsoft SDKs/Windows/*/Bin/x64"
        "${ProgramFilesX86}/Microsoft SDKs/Windows/*/Bin/x64"
        "${ProgramFiles}/Windows Kits/*/bin/*/x64"
        "${ProgramFilesX86}/Windows Kits/*/bin/*/x64"
        "${ProgramFiles}/Windows Kits/10/bin/*/x64"
        "${ProgramFilesX86}/Windows Kits/10/bin/*/x64"
        )
    IF (MC_COMPILER)
        MESSAGE("Windows SDK mc:     found")
    ELSE (MC_COMPILER)
        MESSAGE("Windows SDK mc:     not found")
    ENDIF (MC_COMPILER)
    ADD_SUBDIRECTORY(events.w32)
ENDIF (WIN32)


SET(PCRE2_FLAG "0")
SET(PCRE_FLAG "0")
SET(PCRE_DEFINE "// PCRE is not used")
SET(PCRE2_DEFINE "// PCRE2 is not used")
IF (USE_PCRE)
    FIND_PACKAGE(PCRE2 QUIET)
    IF (PCRE2_FOUND)
        MESSAGE("PCRE2 support:      compile")
        INCLUDE_DIRECTORIES(${PCRE2_INCLUDE_DIR})
        LINK_LIBRARIES(${PCRE2_LIBRARY})
        SET(PCRE2_FLAG "1")
        SET(PCRE2_DEFINE "#define HAVE_PCRE2")
    ELSE (PCRE2_FOUND)
        FIND_PACKAGE(PCRE QUIET)
        IF (PCRE_FOUND)
            MESSAGE("PCRE support:       compile")
            INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR})
            LINK_LIBRARIES(${PCRE_LIBRARY})
            SET(PCRE_FLAG "1")
            SET(PCRE_DEFINE "#define HAVE_PCRE")
        ELSE (PCRE_FOUND)
            MESSAGE(FATAL_ERROR "PCRE library not found")
        ENDIF (PCRE_FOUND)
    ENDIF (PCRE2_FOUND)
ENDIF (USE_PCRE)


SET(ZLIB_FLAG "0")
FIND_PACKAGE(ZLIB QUIET)
SET(ZLIB_DEFINE "// ZLib is not used")
IF (ZLIB_FOUND)
    MESSAGE("ZLIB support:       compile")
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
    LINK_LIBRARIES(${ZLIB_LIBRARY})
    SET(ZLIB_FLAG "1")
    SET(ZLIB_DEFINE "#define HAVE_ZLIB")
ELSE (ZLIB_FOUND)
    MESSAGE("ZLIB support:       don't compile")
    SET(ZLIB_FLAG "0")
ENDIF (ZLIB_FOUND)

FIND_PACKAGE(Brotli QUIET)
SET(BROTLI_DEFINE "// Brolti is not used")
SET(BROTLI_FLAG "0")
IF (USE_BROTLI)
    IF (BROTLI_FOUND)
        MESSAGE("Brotli support:     compile")
        INCLUDE_DIRECTORIES(${BROTLI_INCLUDE_DIR})
        LINK_LIBRARIES(${BROTLI_LIBRARIES})
        SET(BROTLI_FLAG "1")
        SET(BROTLI_DEFINE "#define HAVE_BROTLI")
    ELSE (BROTLI_FOUND)
        MESSAGE("Brotli support:     don't compile")
        SET(BROTLI_FLAG "0")
    ENDIF (BROTLI_FOUND)
ENDIF ()


SET(OPENSSL_FLAG "0")
SET(OPENSSL_DEFINE "// OpenSSL is not used")
IF (USE_OPENSSL)
    FIND_PACKAGE(OpenSSL QUIET)
    IF (OPENSSL_FOUND)
        MESSAGE("OpenSSL support:    compile")
        INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
        LINK_LIBRARIES(${OPENSSL_LIBRARIES})
        SET(OPENSSL_FLAG "1")
        SET(OPENSSL_DEFINE "#define HAVE_OPENSSL")
    ELSE (OPENSSL_FOUND)
        MESSAGE("OpenSSL support: don't compile")
    ENDIF (OPENSSL_FOUND)
ENDIF (USE_OPENSSL)


INCLUDE(cmake/CXX20.cmake)
check_for_cxx20_compiler(CXX20_COMPILER)
# If a C++20 compiler is available, then set the appropriate flags
IF (CXX20_COMPILER)
    enable_cxx20()
    SET(CXX_STANDARD "20")
    MESSAGE("C++ compiler:       20")
    SET(CXX_VERSION_DEFINE "#define CXX_VERSION 20")
ELSE (CXX20_COMPILER)
    INCLUDE(cmake/CXX17.cmake)
    check_for_cxx17_compiler(CXX17_COMPILER)
    # If a C++17 compiler is available, then set the appropriate flags
    IF (CXX17_COMPILER)
        enable_cxx17()
        SET(CXX_STANDARD "17")
        MESSAGE("C++ compiler:       17")
        SET(CXX_VERSION_DEFINE "#define CXX_VERSION 17")
    ELSE (CXX17_COMPILER)
        MESSAGE(FATAL_ERROR "C++ compiler 17 or above isn't available")
    ENDIF (CXX17_COMPILER)
ENDIF (CXX20_COMPILER)

SET(THEMES_PREFIX ${CMAKE_INSTALL_PREFIX})

SET(GTEST_DEFINE "// GTest is not used")
IF (USE_GTEST)
    SET(GTEST_FLAG "1")
    MESSAGE("GTest unit tests:   compile")
    SET(GTEST_DEFINE "#define USE_GTEST")
ELSE ()
    SET(GTEST_FLAG "0")
    MESSAGE("GTest unit tests:   don't compile")
ENDIF ()

SET(BUILD_TEST_WS_DEFINE "// TestWebService is not built")
IF (BUILD_TEST_WS)
    SET(BUILD_TEST_WS "1")
    SET(BUILD_TEST_WS_DEFINE "#define BUILD_TEST_WS")
ELSE ()
    SET(BUILD_TEST_WS "0")
ENDIF ()

INCLUDE_DIRECTORIES(${SPTK_SOURCE_DIR})

CONFIGURE_FILE("${SPTK_SOURCE_DIR}/sptk5/sptk-config.h.cmake" "${SPTK_SOURCE_DIR}/sptk5/sptk-config.h" @ONLY)

ADD_SUBDIRECTORY(sptk5)

IF (USE_GTEST)
    SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY(googletest)
    IF (WIN32)
        TARGET_COMPILE_OPTIONS(gtest PUBLIC "/W2")
    ENDIF ()
    LINK_LIBRARIES(gtest)
ENDIF ()


IF (FLTK_FLAG)
    ADD_SUBDIRECTORY(themes)
ENDIF (FLTK_FLAG)

IF (LIBRARY_TYPE STREQUAL "SHARED")
    ADD_DEFINITIONS(-DSP_LIBRARY)
ENDIF (LIBRARY_TYPE STREQUAL "SHARED")

ADD_SUBDIRECTORY(src)

IF (BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF ()

IF (BUILD_UTILS)
    ADD_SUBDIRECTORY(utilities)
ENDIF ()

IF (USE_GTEST)
    ADD_SUBDIRECTORY(test)
ENDIF ()

IF (BUILD_PACKAGE)
    ADD_SUBDIRECTORY(packages)
ENDIF ()

FILE(GLOB includeFiles "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/*.h")
INSTALL(FILES ${includeFiles} DESTINATION include/sptk5 COMPONENT core)

FILE(GLOB includeFilesXDoc "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/xdoc/*.h")
INSTALL(FILES ${includeFilesXDoc} DESTINATION include/sptk5/xdoc COMPONENT core)

FILE(GLOB includeFilesWSDL "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/wsdl/*.h")
INSTALL(FILES ${includeFilesWSDL} DESTINATION include/sptk5/wsdl COMPONENT core)

FILE(GLOB includeFilesWSDL2 "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/wsdl/protocol/*.h")
INSTALL(FILES ${includeFilesWSDL2} DESTINATION include/sptk5/wsdl/protocol COMPONENT core)

FILE(GLOB includeFilesNet "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/net/*.h")
INSTALL(FILES ${includeFilesNet} DESTINATION include/sptk5/net COMPONENT core)

FILE(GLOB includeFilesThreads "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/threads/*.h")
INSTALL(FILES ${includeFilesThreads} DESTINATION include/sptk5/threads COMPONENT core)

FILE(GLOB includeFilesDB "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/db/*.h")
INSTALL(FILES ${includeFilesDB} DESTINATION include/sptk5/db COMPONENT db)

FILE(GLOB includeFilesDB "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/test/*.h")
INSTALL(FILES ${includeFilesDB} DESTINATION include/sptk5/test)

IF (FLTK_FLAG)
    FILE(GLOB includeFilesGUI "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/gui/*.h")
    INSTALL(FILES ${includeFilesGUI} DESTINATION include/sptk5/gui COMPONENT gui)
ENDIF (FLTK_FLAG)

FILE(GLOB includeFilesGroup "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/c*")
INSTALL(FILES ${includeFilesGroup} DESTINATION include/sptk5 COMPONENT core)

# create uninstall configuration file
CONFIGURE_FILE(
    "${PROJECT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    #  "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
    "share/sptk5/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

# create uninstall target
ADD_CUSTOM_TARGET(
    uninstall
    "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake "
)
