IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

ADD_LIBRARY (spwsdl5 ${LIBRARY_TYPE}
    WSBasicTypes.cpp
    WSComplexType.cpp
    WSConnection.cpp
    WSListener.cpp
    WSMessageIndex.cpp
    WSParser.cpp
    WSParserComplexType.cpp
    WSRequest.cpp
    WSRestriction.cpp
    WSTypeTranslator.cpp
    protocol/WSStaticHttpProtocol.cpp
    protocol/WSWebServiceProtocol.cpp
    protocol/WSWebSocketsProtocol.cpp
)
SET_TARGET_PROPERTIES(spwsdl5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

TARGET_LINK_LIBRARIES(spwsdl5 sputil5 spdb5)

INSTALL(TARGETS spwsdl5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
