# Under Cygwin, CMake 2.8.4 and newer doesn't define WIN32 anymore.
# For other environments, required CMake version can be 2.6 or higher.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# The name of our project is "SPTK".  CMakeLists files in this project can
# refer to the root source directory of the project as ${SPTK_SOURCE_DIR} and
# to the root binary directory of the project as ${SPTK_BINARY_DIR}.
PROJECT (SPTK)


#SET (CMAKE_BUILD_TYPE Release)
SET (CMAKE_BUILD_TYPE Debug)

SET (VERSION "5.2.15")
SET (SOVERSION "5")

SET (CMAKE_ENABLE_COMPILE_COMMANDS ON)

# Place compiled libraries and examples into ./bin/
SET (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

IF (UNIX)
    OPTION (USE_NEW_ABI "Use new GCC ABI" ON)
ENDIF ()

OPTION (USE_FIREBIRD   "Build Firebird support library" ON)
OPTION (USE_FLTK       "Build GUI library" ON)
OPTION (USE_MYSQL      "Build MySQL support library" ON)
OPTION (USE_ODBC       "Build ODBC support library" ON)
OPTION (USE_OPENSSL    "Build OpenSSL support library" ON)
OPTION (USE_ORACLE     "Build Oracle support library" ON)
OPTION (USE_POSTGRESQL "Build PostgreSQL support library" ON)
OPTION (USE_SQLITE3    "Build SQLite3 support library" ON)
OPTION (USE_GTEST      "Build Google Test unit tests" ON)

OPTION (USE_PCRE       "Use PCRE library" ON)

OPTION (BUILD_EXAMPLES "Build usage examples" ON)
OPTION (BUILD_UTILS    "Build utitlity programs" ON)

INCLUDE (${CMAKE_ROOT}/Modules/FindASPELL.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

INCLUDE (cmake/OSType.cmake)

INCLUDE (CheckTypeSize)
check_type_size("void*" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)
SET (BITNESS 32)
IF (SIZEOF_VOID_P EQUAL 8)
  SET (BITNESS 64)
ENDIF()

MESSAGE ("Build type:         ${CMAKE_BUILD_TYPE} ${BITNESS} bit")
MESSAGE ("Compiler:           ${CMAKE_CXX_COMPILER_ID}")
IF (WIN32)
  ADD_DEFINITIONS (-DHAVE_CONFIG_H -DWIN32 -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  LINK_LIBRARIES(wsock32 ole32 uuid comctl32 ws2_32)
ELSE ()
  IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    ADD_DEFINITIONS (
        -Wall -pedantic
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-cast-align
        -Wcovered-switch-default
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
  ELSE()
    ADD_DEFINITIONS (-Wall -pedantic -Wno-long-long -fPIC)
  ENDIF()

  IF (USE_NEW_ABI)
    SET (NEW_ABI_FLAG "1")
    MESSAGE ("GCC ABI:            1")
    ADD_DEFINITIONS (-D_GLIBCXX_USE_CXX11_ABI=1)
  ELSE ()
    # For Oracle client libraries, we need compatibility with old ABI
    SET (NEW_ABI_FLAG "0")
    MESSAGE ("GCC ABI:            0")
    ADD_DEFINITIONS (-D_GLIBCXX_USE_CXX11_ABI=0)
  ENDIF ()

  IF (UNIX)
    LINK_LIBRARIES(pthread rt dl)
  ELSE ()
    LINK_LIBRARIES(pthread rt)
  ENDIF ()
ENDIF ()

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
  ADD_DEFINITIONS (-DSPTK_LOCK_DEBUG)
ENDIF()

IF (WIN32)
  SET (LIBRARY_TYPE STATIC)
  #SET (LIBRARY_TYPE SHARED)
  IF (LIBRARY_TYPE STREQUAL "SHARED")
    ADD_DEFINITIONS (-DSP_DLL)
  ENDIF(LIBRARY_TYPE STREQUAL "SHARED")
ELSE (WIN32)
  #SET (LIBRARY_TYPE STATIC)
  SET (LIBRARY_TYPE SHARED)
ENDIF (WIN32)
MESSAGE ("Library type:       " ${LIBRARY_TYPE})


SET (ODBC_FLAG "0")
IF (USE_ODBC)
  IF (WIN32)
    SET (ODBC_FLAG "1")
    SET (ODBC_LIBRARY "odbc32")
  ELSE (WIN32)
    FIND_LIBRARY (ODBC_LIBRARY NAMES odbc PATHS /usr/lib /usr/lib/* /usr/local/lib)
  ENDIF (WIN32)

  IF (ODBC_LIBRARY)
    SET (ODBC_FLAG "1")
  ENDIF (ODBC_LIBRARY)
ENDIF (USE_ODBC)

IF (ODBC_FLAG)
  MESSAGE("ODBC support:       compile")
ELSE (ODBC_FLAG)
  MESSAGE("ODBC support:       don't compile")
  SET (ODBC_LIBRARY "")
ENDIF (ODBC_FLAG)

FIND_LIBRARY (SOCKET_LIBRARY NAMES socket PATHS /usr/lib)
IF (NOT SOCKET_LIBRARY)
  SET (SOCKET_LIBRARY "")
ELSE (NOT SOCKET_LIBRARY)
  SET (SOCKET_LIBRARY "-lsocket -lrt -lnsl")
ENDIF (NOT SOCKET_LIBRARY)

SET (POSTGRESQL_FLAG "0")
IF (USE_POSTGRESQL)
  FIND_PACKAGE(PostgreSQL QUIET)
  IF (PostgreSQL_FOUND)
    MESSAGE("PostgreSQL support: compile")
    SET (POSTGRESQL_FLAG "1")
  ELSE (PostgreSQL_FOUND)
    MESSAGE("PostgreSQL support: don't compile")
  ENDIF (PostgreSQL_FOUND)
ENDIF (USE_POSTGRESQL)

SET (FIREBIRD_FLAG "0")
IF (USE_FIREBIRD)
  FIND_PACKAGE(Firebird QUIET)
  IF (Firebird_FOUND)
    MESSAGE("Firebird support:   compile")
    SET (FIREBIRD_FLAG "1")
  ELSE (Firebird_FOUND)
    MESSAGE("Firebird support:   don't compile")
  ENDIF (Firebird_FOUND)
ENDIF (USE_FIREBIRD)


SET (ORACLE_FLAG "0")
IF (USE_ORACLE)
  FIND_PACKAGE(Oracle QUIET)
  IF (Oracle_FOUND)
    MESSAGE("Oracle support:     compile")
    SET (ORACLE_FLAG "1")
  ELSE (Oracle_FOUND)
    MESSAGE("Oracle support:     don't compile")
  ENDIF (Oracle_FOUND)
ENDIF (USE_ORACLE)


SET (MYSQL_FLAG "0")
IF (USE_MYSQL)
  FIND_PACKAGE(MySQL QUIET)
  IF (MySQL_FOUND)
    MESSAGE("MySQL support:      compile")
    SET (MYSQL_FLAG "1")
  ELSE (MySQL_FOUND)
    MESSAGE("MySQL support:      don't compile")
  ENDIF (MySQL_FOUND)
ENDIF (USE_MYSQL)


SET (SQLITE3_FLAG "0")
IF (USE_SQLITE3)
  FIND_LIBRARY (SQLITE3_LIBRARY NAMES sqlite3 PATHS /usr/lib /usr/local/lib)
  IF (NOT SQLITE3_LIBRARY AND WIN32)
    FIND_FILE (SQLITE3_LIBRARY sqlite3.dll PATHS /usr/lib /usr/local/lib)
  ENDIF (NOT SQLITE3_LIBRARY AND WIN32)
  IF (SQLITE3_LIBRARY)
    SET (SQLITE3_FLAG "1")
  ELSE (SQLITE3_LIBRARY)
    SET (SQLITE3_LIBRARY "")
  ENDIF (SQLITE3_LIBRARY)
ENDIF (USE_SQLITE3)

IF (SQLITE3_FLAG)
  MESSAGE("SQLITE3 support:    compile")
ELSE (SQLITE3_FLAG)
  MESSAGE("SQLITE3 support:    don't compile")
ENDIF (SQLITE3_FLAG)


# Detect FLTK and Aspell
SET (FLTK_FLAG "0")
SET (ASPELL_FLAG "0")
IF (USE_FLTK)
    FIND_PACKAGE (FLTK13 QUIET)
    IF (FLTK_FOUND)
        SET (FLTK_FLAG "1")
        #ADD_DEFINITIONS (${FLTK_CXX_FLAGS})
    ENDIF (FLTK_FOUND)
    IF (FLTK_FOUND)
        FIND_PACKAGE (ASPELL)
        IF (ASPELL_FOUND)
            SET (ASPELL_FLAG "1")
        ELSE (ASPELL_FOUND)
            SET (ASPELL_LIBRARIES "")
        ENDIF (ASPELL_FOUND)

        IF (NOT WIN32)
            FIND_PACKAGE (PNG)
            IF (NOT PNG_FOUND)
                MESSAGE(FATAL_ERROR "Please istall libpng (devel)")
            ENDIF()

            FIND_PACKAGE (X11)
            IF (NOT X11_Xshape_FOUND)
                MESSAGE(FATAL_ERROR "Please istall libXext (devel)")
            ENDIF()
        ENDIF()
    ENDIF (FLTK_FOUND)
ENDIF (USE_FLTK)

IF (FLTK_FOUND)
  MESSAGE("FLTK support:       compile")

  IF (ASPELL_FLAG)
    MESSAGE("ASPELL support:     compile")
  ELSE (ASPELL_FLAG)
    MESSAGE("ASPELL support:     don't compile")
  ENDIF (ASPELL_FLAG)

ELSE (FLTK_FOUND)
  MESSAGE("FLTK support:       don't compile")
ENDIF (FLTK_FOUND)


IF (WIN32)
  set(PROGRAM_FILES_86 "PROGRAMFILES(X86)")
  # Windows message resource compiler (included with Windows SDK)
  FIND_PROGRAM (MC_COMPILER mc PATHS
                    "$ENV{ProgramFiles}/Windows Kits/*/bin/x64"
                    "$ENV{ProgramW6432}/Windows Kits/*/bin/x64"
                    "$ENV{ProgramFiles}/Microsoft SDKs/Windows/*/Bin/x64"
                    "$ENV{ProgramW6432}/Microsoft SDKs/Windows/*/Bin/x64"
                    "$ENV{ProgramFiles}/Windows Kits/*/bin/*/x64"
                    "$ENV{ProgramW6432}/Windows Kits/*/bin/*/x64"
                    "$ENV{ProgramFiles}/Windows Kits/10/bin/10.0.15063.0/x64"
                    "$ENV{${PROGRAM_FILES_86}}/Windows Kits/*/bin/*/x64"
                    "$ENV{${PROGRAM_FILES_86}}/Windows Kits/*/bin/*/x86"
                    )
  IF (MC_COMPILER)
    MESSAGE("Windows SDK mc:     found")
  ELSE (MC_COMPILER)
    MESSAGE("Windows SDK mc:     not found")
  ENDIF (MC_COMPILER)
  ADD_SUBDIRECTORY (events.w32)
ENDIF (WIN32)


SET (PCRE_FLAG "0")
IF (USE_PCRE)
  FIND_PACKAGE(PCRE QUIET)
  IF (PCRE_FOUND)
    MESSAGE("PCRE support:       compile")
    INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR})
    LINK_LIBRARIES(${PCRE_LIBRARY})
    SET (PCRE_FLAG "1")
  ELSE (PCRE_FOUND)
    MESSAGE(FATAL_ERROR "PCRE library not found")
  ENDIF (PCRE_FOUND)
ENDIF (USE_PCRE)

SET (ZLIB_FLAG "0")
FIND_PACKAGE(ZLIB QUIET)
IF (ZLIB_FOUND)
  MESSAGE("ZLIB support:       compile")
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
  LINK_LIBRARIES(${ZLIB_LIBRARY})
  SET (ZLIB_FLAG "1")
ELSE (ZLIB_FOUND)
  MESSAGE("ZLIB support:       don't compile")
  SET (ZLIB_FLAG "0")
ENDIF (ZLIB_FOUND)

SET (OPENSSL_FLAG "0")
IF (USE_OPENSSL)
  FIND_PACKAGE(OpenSSL QUIET)
  IF (OPENSSL_FOUND)
    MESSAGE("OpenSSL support:    compile")
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    LINK_LIBRARIES(${OPENSSL_LIBRARIES})
    SET (OPENSSL_FLAG "1")
  ELSE (OPENSSL_FOUND)
    MESSAGE("OpenSSL support:    don't compile")
  ENDIF (OPENSSL_FOUND)
ENDIF (USE_OPENSSL)

INCLUDE(cmake/CXX17.cmake)
check_for_cxx17_compiler(CXX17_COMPILER)
# If a C++17 compiler is available, then set the appropriate flags
IF (CXX17_COMPILER)
  enable_cxx17()
  SET (CXX17_FLAG "1")
ELSE (CXX17_COMPILER)
  MESSAGE(FATAL_ERROR "C++17 compiler is not available")
ENDIF (CXX17_COMPILER)

SET (THEMES_PREFIX ${CMAKE_INSTALL_PREFIX})

INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})

IF (USE_GTEST)
  SET (GTEST_FLAG "1")
ELSE ()
  SET (GTEST_FLAG "0")
ENDIF ()

CONFIGURE_FILE ("${SPTK_SOURCE_DIR}/sptk5/sptk-config.h.cmake" "${SPTK_SOURCE_DIR}/sptk5/sptk-config.h" @ONLY)

IF (GTEST_FLAG)
  SET (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  ADD_SUBDIRECTORY (googletest)
  IF (WIN32)
    TARGET_COMPILE_OPTIONS(gtest PUBLIC "/W2")
  ENDIF ()
  LINK_LIBRARIES (gtest)
ENDIF ()

IF (FLTK_FLAG)
  ADD_SUBDIRECTORY (themes)
ENDIF (FLTK_FLAG)

ADD_SUBDIRECTORY (src)

IF (BUILD_EXAMPLES)
  ADD_SUBDIRECTORY (examples)
ENDIF ()

IF (BUILD_UTILS)
  ADD_SUBDIRECTORY (utilities)
ENDIF ()

FILE (GLOB includeFiles "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/*.h")
INSTALL (FILES ${includeFiles} DESTINATION include/sptk5)

FILE (GLOB includeFilesJSON "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/json/*.h")
INSTALL (FILES ${includeFilesJSON} DESTINATION include/sptk5/json)

FILE (GLOB includeFilesXML "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/xml/*.h")
INSTALL (FILES ${includeFilesXML} DESTINATION include/sptk5/xml)

FILE (GLOB includeFilesWSDL "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/wsdl/*.h")
INSTALL (FILES ${includeFilesWSDL} DESTINATION include/sptk5/wsdl)

FILE (GLOB includeFilesNet "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/net/*.h")
INSTALL (FILES ${includeFilesNet} DESTINATION include/sptk5/net)

FILE (GLOB includeFilesThreads "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/threads/*.h")
INSTALL (FILES ${includeFilesThreads} DESTINATION include/sptk5/threads)

FILE (GLOB includeFilesDB "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/db/*.h")
INSTALL (FILES ${includeFilesDB} DESTINATION include/sptk5/db)

FILE (GLOB includeFilesDB "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/test/*.h")
INSTALL (FILES ${includeFilesDB} DESTINATION include/sptk5/test)

IF (FLTK_FLAG)
  FILE (GLOB includeFilesGUI "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/gui/*.h")
  INSTALL (FILES ${includeFilesGUI} DESTINATION include/sptk5/gui)
ENDIF (FLTK_FLAG)

FILE (GLOB includeFilesGroup "${CMAKE_CURRENT_SOURCE_DIR}/sptk5/c*")
INSTALL (FILES ${includeFilesGroup} DESTINATION include/sptk5)

IF (POSTGRESQL_FLAG)
  FILE (GLOB pgSQLscripts "${CMAKE_CURRENT_SOURCE_DIR}/dbtools/sql2cpp")
  INSTALL (PROGRAMS ${pgSQLscripts} DESTINATION bin)
  FILE (GLOB pgSQLscriptConfigs "${CMAKE_CURRENT_SOURCE_DIR}/dbtools/*.conf")
  INSTALL (FILES ${pgSQLscriptConfigs} DESTINATION share/sptk5/doc)
ENDIF (POSTGRESQL_FLAG)

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
  "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
)
