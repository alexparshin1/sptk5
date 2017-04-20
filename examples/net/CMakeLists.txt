INCLUDE_DIRECTORIES (${SPTK_SOURCE_DIR})
INCLUDE_DIRECTORIES (/usr/local/include)

LINK_DIRECTORIES (${SPTK_BINARY_DIR})

SET (NET_TESTS ws_server_test tcp_client_test tcp_server_test smtp_connect udp_client_test udp_server_test ssl_server_test ssl_client_test)

IF (FLTK_FLAG)
    INCLUDE_DIRECTORIES (${FLTK_INCLUDE_DIR})
    LINK_DIRECTORIES (${FLTK_LIB_DIR})
    SET (NET_TESTS ${NET_TESTS} imap_connect http_connect)
    LINK_LIBRARIES (sputil5 sptk5 spwsdl5 spdb5 ${SOCKET_LIBRARY} ${FLTK_LIBRARIES} ${ASPELL_LIBRARIES})
ELSE (FLTK_FLAG)
    LINK_LIBRARIES (sputil5 spwsdl5 spdb5 ${SOCKET_LIBRARY})
ENDIF (FLTK_FLAG)

IF (UNIX)
    LINK_LIBRARIES (pthread rt)
ENDIF (UNIX)

FOREACH (NET_TEST ${NET_TESTS})
    ADD_EXECUTABLE (${NET_TEST} ${NET_TEST}.cpp)
ENDFOREACH (NET_TEST)

