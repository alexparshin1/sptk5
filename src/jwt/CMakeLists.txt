set (TARGET_NAME spjwt)

option (WITHOUT_OPENSSL "Use GnuTLS for encryption instead of OpenSSL" OFF)

if (UNIX)
    option (ENABLE_PIC "Use position independent code in static library build." OFF)
endif ()

if (BUILD_SHARED_LIBS)
    set (LIBRARY_TYPE SHARED)
else ()
    set (LIBRARY_TYPE STATIC JWT.cpp)
    if (MSVC)
        set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")
        set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /Z7")
        if (${ENABLE_DEBUG_INFO_IN_RELEASE})
            set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Z7")
        endif ()
    endif ()
endif ()

    if (MSVC AND STATIC_RUNTIME)
        set (OPENSSL_MSVC_STATIC_RT TRUE)
    endif ()
    find_package (OpenSSL REQUIRED)
    set (FILES_TO_REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/jwt-gnutls.cpp)
    set (SSL_LIBRARY_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR})
    if (MSVC)
        set (SSL_LIBRARIES_DEBUG ${LIB_EAY_DEBUG} ${SSL_EAY_DEBUG})
        set (SSL_LIBRARIES_OPTIMIZED ${LIB_EAY_RELEASE} ${SSL_EAY_RELEASE})
    else ()
        set (SSL_LIBRARIES_DEBUG ${OPENSSL_CRYPTO_LIBRARY})
        set (SSL_LIBRARIES_OPTIMIZED ${OPENSSL_CRYPTO_LIBRARY})
    endif ()

write_file(${CMAKE_CURRENT_BINARY_DIR}/config.h "")

file (GLOB SOURCE_FILES "../include/*.h" "*.h" "*.cpp")
list (REMOVE_ITEM SOURCE_FILES ${FILES_TO_REMOVE})

add_library (${TARGET_NAME} ${LIBRARY_TYPE} ${SOURCE_FILES})

target_include_directories (${TARGET_NAME} PRIVATE
    ${SSL_LIBRARY_INCLUDE_DIR}
    ${JANSSON_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

if (UNIX AND ENABLE_LTO)
    set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()

if (MSVC)
    target_compile_definitions (${TARGET_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    if (BUILD_SHARED_LIBS)
        target_compile_definitions (${TARGET_NAME}
            PUBLIC JWT_DLL_CONFIG
            PRIVATE JWT_BUILD_SHARED_LIBRARY
            )
    endif ()
endif ()

if (NOT BUILD_SHARED_LIBS AND ENABLE_PIC)
    target_compile_options (${TARGET_NAME} PRIVATE -fPIC)
endif ()

target_link_libraries (${TARGET_NAME}
    debug ${SSL_LIBRARIES_DEBUG} optimized ${SSL_LIBRARIES_OPTIMIZED}
    ${JANSSON_LIBRARIES}
    )

install (TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )
