IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

ADD_LIBRARY (spwsdl5 ${LIBRARY_TYPE}
    SourceModule.cpp
    WSBasicTypes.cpp
    WSComplexType.cpp
    WSConnection.cpp
    WSListener.cpp
    WSMessageIndex.cpp
    WSParser.cpp
    WSParserComplexType.cpp
    WSRequest.cpp
    WSRestriction.cpp
    WSServices.cpp
    WSTypeTranslator.cpp
    protocol/BaseWebServiceProtocol.cpp
    protocol/WSProtocol.cpp
    protocol/WSStaticHttpProtocol.cpp
    protocol/WSWebServiceProtocol.cpp
    protocol/WSWebSocketsProtocol.cpp
    OpenApiGenerator.cpp
)
SET_TARGET_PROPERTIES(spwsdl5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

TARGET_LINK_LIBRARIES(spwsdl5 sputil5 spdb5)

INSTALL(
    TARGETS spwsdl5
    RUNTIME DESTINATION bin COMPONENT core
    LIBRARY DESTINATION lib COMPONENT core
    ARCHIVE DESTINATION lib COMPONENT core)
