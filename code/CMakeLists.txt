# ╔══════════════════════════════════════════════════════════════════════════════╗
# ║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
# ╟──────────────────────────────────────────────────────────────────────────────╢
# ║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
# ║  email                alexeyp@gmail.com                                      ║
# ╚══════════════════════════════════════════════════════════════════════════════╝
# ┌──────────────────────────────────────────────────────────────────────────────┐
# │   This library is free software; you can redistribute it and/or modify it    │
# │   under the terms of the GNU Library General Public License as published by  │
# │   the Free Software Foundation; either version 2 of the License, or (at your │
# │   option) any later version.                                                 │
# │                                                                              │
# │   This library is distributed in the hope that it will be useful, but        │
# │   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
# │   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
# │   General Public License for more details.                                   │
# │                                                                              │
# │   You should have received a copy of the GNU Library General Public License  │
# │   along with this library; if not, write to the Free Software Foundation,    │
# │   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
# │                                                                              │
# │   Please report all bugs and problems to alexeyp@gmail.com.                  │
# └──────────────────────────────────────────────────────────────────────────────┘

CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

set(TARGET_MESSAGES OFF)
set(CMAKE_TARGET_MESSAGES OFF)
set(CMAKE_INSTALL_MESSAGE NEVER)

SET(VERSION_MAJOR "5")
SET(VERSION_MINOR "6")
SET(VERSION_PATCH "9")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

PROJECT(SPTK
        VERSION ${VERSION}
        LANGUAGES CXX C)
SET(SOVERSION "5")

IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF ()

IF (CMAKE_VERSION VERSION_GREATER "3.31.0")
    CMAKE_POLICY(SET CMP0177 NEW)
ENDIF ()

FUNCTION(SET_SPTK_TARGET_PROPERTIES target)
    SET_TARGET_PROPERTIES(${target} PROPERTIES
            SOVERSION ${SOVERSION}
            VERSION ${PROJECT_VERSION})
    # Where an installed library looks for the SPTK libraries it needs: beside itself, whatever
    # directory that turns out to be. It cannot be left empty, and it must not be CMAKE_INSTALL_RPATH
    # either - that is set below to this build tree's lib/, which would bake a path from the build
    # machine into every shipped library.
    #
    # Empty was not harmless. A library's own search path is not inherited from whatever loaded it,
    # so with nothing here libspwsdl5 found libspdb5 through the loader cache instead of beside
    # itself: an application shipping SPTK alongside it, in its own directory, still loaded whatever
    # libspdb5 happened to be installed on the machine. It is also what dlopen uses when spdb5 goes
    # looking for a database driver by name, so without it the drivers are only ever found in the
    # system directories.
    IF (APPLE)
        SET(SPTK_TARGET_RPATH "@loader_path")
    ELSEIF (UNIX)
        SET(SPTK_TARGET_RPATH "$ORIGIN")
    ENDIF ()

    # Keep the compiler's own libstdc++ directory on the RPATH too - see the BSD/GCC note further
    # down, which pins it in CMAKE_INSTALL_RPATH. Setting INSTALL_RPATH on a target *replaces*
    # CMAKE_INSTALL_RPATH rather than adding to it, so without this the pinned directory is
    # silently dropped from everything installed, and the loader falls back to the system-wide
    # search order - picking up whichever libstdc++ sorts first, which may be too old for the
    # GLIBCXX symbols this build needs.
    IF (SPTK_TARGET_RPATH AND GCC_LIBSTDCXX_DIR)
        SET(SPTK_TARGET_RPATH "${SPTK_TARGET_RPATH};${GCC_LIBSTDCXX_DIR}")
    ENDIF ()

    IF (SPTK_TARGET_RPATH)
        SET_TARGET_PROPERTIES(${target} PROPERTIES INSTALL_RPATH "${SPTK_TARGET_RPATH}")
    ENDIF ()
ENDFUNCTION()

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Place compiled libraries and examples into ./bin/
SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

SET(TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
SET(CMAKE_INSTALL_RPATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

IF (UNIX)
    OPTION(USE_NEW_ABI "Use new GCC ABI" ON)
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
OPTION(USE_GTEST "Build SPTK unit tests" ON)
OPTION(USE_MSSQL_TESTS "Enable MSSQL-dependent unit tests (requires a live MSSQL DSN)" OFF)

OPTION(USE_PCRE "Use PCRE library" ON)
OPTION(USE_BROTLI "Use Brotli library" ON)

OPTION(BUILD_EXAMPLES "Build usage examples" ON)
OPTION(BUILD_UTILS "Build utility programs" ON)
OPTION(BUILD_PACKAGE "Build OS package" ON)
OPTION(BUILD_INSTALLER "Build SP Installer." ON)

IF (NOT WIN32)
    OPTION(BUILD_TEST_WS "Build Test Web Service" ON)
    OPTION(BUILD_WITH_COVERAGE "Build with coverage" OFF)
ENDIF ()

INCLUDE(${CMAKE_ROOT}/Modules/FindASPELL.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

INCLUDE(cmake/OSType.cmake)

# On BSD, multiple GCC versions can be installed side by side, each registering its own
# libstdc++.so.6 with the system's ldconfig. The search order there is alphabetical over
# /usr/local/lib/*, which does not necessarily favor the version this project is actually
# built with, so a binary can end up loading a libstdc++ too old for the GLIBCXX symbols it
# needs. Pin the RPATH to the compiler's own libstdc++ directory to avoid depending on that
# system-wide search order.
IF (BSD AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    EXECUTE_PROCESS(
            COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so
            OUTPUT_VARIABLE GCC_LIBSTDCXX_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    GET_FILENAME_COMPONENT(GCC_LIBSTDCXX_DIR "${GCC_LIBSTDCXX_PATH}" DIRECTORY)
    GET_FILENAME_COMPONENT(GCC_LIBSTDCXX_DIR "${GCC_LIBSTDCXX_DIR}" REALPATH)
    IF (GCC_LIBSTDCXX_DIR AND EXISTS "${GCC_LIBSTDCXX_DIR}")
        MESSAGE("GCC libstdc++ dir:  ${GCC_LIBSTDCXX_DIR}")
        SET(CMAKE_BUILD_RPATH "${GCC_LIBSTDCXX_DIR}")
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:${GCC_LIBSTDCXX_DIR}")
    ENDIF ()
ENDIF ()

INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)
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
    ADD_COMPILE_OPTIONS(/wd4275 /std:c++20 /W4 /arch:AVX2)
    LINK_LIBRARIES(wsock32 ole32 uuid comctl32 ws2_32 winhttp crypt32 secur32 shlwapi bcrypt)
ELSE ()
    IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # std::jthread and <stop_token> are still behind libc++'s experimental gate in LLVM 19,
        # and Thread - the base of every thread in this library - is built on both. Without this
        # the whole library fails to compile against libc++, which on FreeBSD is what the base
        # compiler uses. Both a compile and a link option: it selects the declarations and it
        # brings in libc++experimental.
        #
        # Note that it puts an ABI tag naming the libc++ version (abi:se190107 for 19.1.7) on
        # those symbols, so everything in the process has to be built the same way, and a libc++
        # upgrade is an ABI break for them. That is the price of the flag; the alternative is to
        # stop using the two types.
        ADD_COMPILE_OPTIONS(-fexperimental-library)
        ADD_LINK_OPTIONS(-fexperimental-library)

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
        )
        IF (BUILD_WITH_COVERAGE)
            ADD_DEFINITIONS(-fprofile-instr-generate -fcoverage-mapping)
        ENDIF ()
    ELSE ()
        ADD_DEFINITIONS(
                -fPIC
                -Wall -pedantic
                -Wno-long-long
                -Wno-restrict
        )
        IF (BUILD_WITH_COVERAGE)
            ADD_DEFINITIONS(--coverage)
        ENDIF ()
    ENDIF ()

    IF (USE_NEW_ABI)
        SET(NEW_ABI_FLAG "1")
        SET(NEW_ABI_DEFINE "#define USE_NEW_ABI")
        MESSAGE("GCC ABI:            1")
    ELSE ()
        # For Oracle client libraries, we need compatibility with old ABI
        SET(NEW_ABI_FLAG "0")
        SET(NEW_ABI_DEFINE "// New C++ ABI not used")
        MESSAGE("GCC ABI:            0")
        ADD_DEFINITIONS(-D_GLIBCXX_USE_CXX11_ABI=0)
    ENDIF ()

    ADD_DEFINITIONS(-DOPENSSL_SUPPRESS_DEPRECATED)

    # Keep inline functions out of the dynamic symbol tables.
    #
    # Without this, each shared library exports its own weak copy of inline definitions such as
    # std::mutex::lock(). The first library loaded then interposes that symbol for every other
    # one, so a lock can be taken by one library's copy while the matching unlock is inlined into
    # another. On FreeBSD the two copies disagree about libstdc++'s __gthread_active() flag -
    # the interposing copy locks for real, the inlined unlock compiles to nothing - and the mutex
    # is left held. The next acquisition then fails with EDEADLK ("Resource deadlock avoided").
    ADD_DEFINITIONS(-fvisibility-inlines-hidden)

    IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
        #ADD_DEFINITIONS(-O1 -msse4.2)

        # Enable address sanitizer:
        #ADD_DEFINITIONS(-fsanitize=address)
        #LINK_LIBRARIES(tsan)

        # Enable thread sanitizer:
        #ADD_DEFINITIONS(-fsanitize=thread)
        #LINK_LIBRARIES(tsan)
    ELSE ()
        ADD_DEFINITIONS(-O3 -msse4.2)
    ENDIF ()

    IF (UNIX)
        LINK_LIBRARIES(pthread rt dl)
        IF (BUILD_WITH_COVERAGE)
            LINK_LIBRARIES(gcov)
        ENDIF ()
    ELSE ()
        LINK_LIBRARIES(pthread rt)
    ENDIF ()
ENDIF ()

IF (WIN32)
    SET(WINDIR "$ENV{ProgramW6432}")
    SET(DATADIR "$ENV{ProgramData}")
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

    # Default to a user-owned install prefix instead of CMake's built-in /usr/local.
    # Installing to /usr/local normally requires root; on BSD, recursively chown'ing
    # /usr/local to reclaim it strips setuid bits from pkg-installed binaries there
    # (e.g. sudo) and can leave the system unable to escalate to root. Only applies
    # when the user hasn't explicitly passed -DCMAKE_INSTALL_PREFIX. Packaging
    # (packages/CMakeLists.txt) overrides this back to a real system prefix.
    IF (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        SET(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Cmake prefix" FORCE)
    ENDIF ()
ENDIF (WIN32)
MESSAGE("Library type:       " ${LIBRARY_TYPE})
MESSAGE("Install prefix:     " ${CMAKE_INSTALL_PREFIX})

INCLUDE_DIRECTORIES(".")
INCLUDE_DIRECTORIES(${SPTK_SOURCE_DIR})

# The build tree too, for the one generated header: sptk5/sptk-config.h is written into
# <build>/sptk5 rather than into the sources, and is included as <sptk5/sptk-config.h> like any
# other. Listed after the source directory only because nothing in the sources shadows it - the
# tracked copy was deleted when the header moved here.
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

SET(SPTK_INCLUDE_DIR ${SPTK_SOURCE_DIR}/sptk5)


SET(ODBC_FLAG "0")
IF (USE_ODBC)
    IF (WIN32)
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
    IF (WIN32)
        SET(EPOLL_FLAG "1")
        # Using built-in wepoll.cpp
        SET(EPOLL_DEFINE "#define HAVE_EPOLL")
    ELSE ()
        FIND_PACKAGE(EPoll QUIET)
        IF (EPOLL_FOUND)
            SET(EPOLL_FLAG "1")
            INCLUDE_DIRECTORIES(${EPOLL_INCLUDE_DIR})
            LINK_LIBRARIES(${EPOLL_LIBRARY})
            SET(EPOLL_DEFINE "#define HAVE_EPOLL")
        ENDIF (EPOLL_FOUND)
    ENDIF ()
ENDIF (USE_EPOLL)


SET(ORACLE_FLAG "0")
SET(ORACLE_DEFINE "// Oracle SQL is not used")
IF (USE_ORACLE)

    FIND_PACKAGE(Oracle QUIET)
    IF (Oracle_FOUND)
        MESSAGE("Oracle client:      found")

        FIND_PACKAGE(OCI QUIET)
        IF (OCI_FOUND)
            MESSAGE("OracleOCI support:  compile")
            SET(ORACLE_OCI_FLAG "1")
            SET(ORACLE_OCI_DEFINE "#define HAVE_ORACLE_OCI")
            SET(ORACLE_FLAG "1")
        ELSE ()
            MESSAGE("OracleOCI support:  don't compile")
        ENDIF ()
    ELSE (Oracle_FOUND)
        MESSAGE("Oracle client:      not found")
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
    IF (WIN32)
        SET(SQLITE3_LIBRARY sqlite3)
        INCLUDE_DIRECTORIES(sqlite3)
        SET(SQLITE3_FLAG "1")
    ELSE ()
        FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 PATHS /usr/lib /usr/local/lib)
        IF (NOT SQLITE3_LIBRARY AND WIN32)
            FIND_FILE(SQLITE3_LIBRARY sqlite3.dll PATHS /usr/lib /usr/local/lib)
        ENDIF ()
        IF (SQLITE3_LIBRARY)
            SET(SQLITE3_FLAG "1")
        ELSE ()
            SET(SQLITE3_LIBRARY "")
        ENDIF ()
    ENDIF ()
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
                MESSAGE(FATAL_ERROR "Please install libpng (devel)")
            ENDIF ()

            FIND_PACKAGE(X11 QUIET)
            IF (NOT X11_Xshape_FOUND)
                MESSAGE(FATAL_ERROR "Please install libXext (devel)")
            ENDIF ()
        ENDIF ()
    ENDIF (FLTK_FOUND)
ENDIF (USE_FLTK)


IF (FLTK_FOUND)
    IF (WIN32)
        SET(CAIRO_FLAG "1")
    ELSE ()
        FIND_PACKAGE(Cairo)
        IF (CAIRO_FOUND)
            MESSAGE("Cairo support:      compile")
            INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIR})
            LINK_LIBRARIES(${CAIRO_LIBRARY})
        ELSE (CAIRO_FOUND)
            MESSAGE("Cairo support:      don't compile")
        ENDIF (CAIRO_FOUND)
    ENDIF ()
ENDIF ()


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
        MESSAGE(FATAL_ERROR "PCRE2 support:      don't compile")
    ENDIF (PCRE2_FOUND)
ENDIF (USE_PCRE)


SET(ZLIB_FLAG "0")
FIND_PACKAGE(ZLIB)
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
SET(BROTLI_DEFINE "// Brotli is not used")
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
CHECK_FOR_CXX20_COMPILER(CXX20_COMPILER)
# If a C++20 compiler is available, then set the appropriate flags
IF (CXX20_COMPILER)
    ENABLE_CXX20()
    SET(CXX_STANDARD "20")
ELSE (CXX20_COMPILER)
    MESSAGE(FATAL_ERROR "C++ compiler 20 or above isn't available")
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

SET(MSSQL_TESTS_DEFINE "// MSSQL unit tests are not enabled")
IF (USE_MSSQL_TESTS AND USE_ODBC)
    MESSAGE("MSSQL unit tests:   compile")
    SET(MSSQL_TESTS_DEFINE "#define HAVE_MSSQL_TESTS")
ELSE ()
    MESSAGE("MSSQL unit tests:   don't compile")
ENDIF ()

SET(BUILD_TEST_WS_DEFINE "// TestWebService is not built")
IF (BUILD_TEST_WS)
    SET(BUILD_TEST_WS "1")
    SET(BUILD_TEST_WS_DEFINE "#define BUILD_TEST_WS")
ELSE ()
    SET(BUILD_TEST_WS "0")
ENDIF ()

INCLUDE_DIRECTORIES(${SPTK_SOURCE_DIR})

# On BSD, the system/pkg googletest is normally built with the base Clang/libc++ toolchain,
# which is ABI-incompatible with a GCC/libstdc++ build of this project (undefined references
# at link time). Build googletest from the vendored source in that combination instead, same
# as Windows (which has no system gtest to link against at all).
SET(BUILD_VENDORED_GTEST OFF)
IF (WIN32 OR (BSD AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    SET(BUILD_VENDORED_GTEST ON)
ENDIF ()

IF (USE_GTEST AND BUILD_VENDORED_GTEST)
    INCLUDE_DIRECTORIES(googletest/googletest/include)
    # A CMake target, so the link resolves by itself.
    SET(SPTK_GTEST_LIBRARY gtest)
ELSEIF (USE_GTEST)
    # The system googletest, located and linked by its full path rather than as a bare -lgtest.
    # FreeBSD's base toolchain does not search /usr/local/lib, where the package installs it, so
    # the bare name fails to resolve there - on Linux the same line happens to work.
    FIND_PATH(SPTK_GTEST_INCLUDE_DIR gtest/gtest.h HINTS /usr/local/include /usr/include)
    FIND_LIBRARY(SPTK_GTEST_LIBRARY NAMES gtest HINTS /usr/local/lib /usr/lib /usr/lib64)
    IF (SPTK_GTEST_INCLUDE_DIR)
        INCLUDE_DIRECTORIES(${SPTK_GTEST_INCLUDE_DIR})
    ENDIF ()
    IF (NOT SPTK_GTEST_LIBRARY)
        MESSAGE(FATAL_ERROR "USE_GTEST is on but no googletest library was found. "
                            "Install one, or turn USE_GTEST off.")
    ENDIF ()
ENDIF ()

# Into the build tree, not back into the sources. What it holds depends on the installation prefix
# and on which optional libraries were found, so writing it into the source tree meant an ordinary
# developer build left that developer's paths in a tracked file, waiting to be committed by whoever
# did not look at the diff. Each build tree now has its own, which is also what lets two of them be
# configured differently without fighting over one file.
CONFIGURE_FILE("${SPTK_SOURCE_DIR}/sptk5/sptk-config.h.cmake" "${CMAKE_BINARY_DIR}/sptk5/sptk-config.h" @ONLY)

ADD_SUBDIRECTORY(sptk5)

IF (SQLITE3_FLAG AND WIN32)
    ADD_SUBDIRECTORY(sqlite3)
ENDIF ()


IF (USE_GTEST AND BUILD_VENDORED_GTEST)
    SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY(googletest)
    IF (WIN32)
        TARGET_COMPILE_OPTIONS(gtest PUBLIC "/W2")
    ENDIF ()
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

IF (FLTK_FLAG AND BUILD_INSTALLER)
    ADD_SUBDIRECTORY(installer)
ENDIF ()

FILE(GLOB includeFiles "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/*.h")
INSTALL(FILES ${includeFiles} DESTINATION include/sptk5 COMPONENT core)

# Installed separately: the glob above no longer reaches it, now that it is generated into the
# build tree. Without this the installed headers are missing the one every other one includes.
INSTALL(FILES "${CMAKE_BINARY_DIR}/sptk5/sptk-config.h" DESTINATION include/sptk5 COMPONENT core)

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
