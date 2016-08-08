IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

ADD_LIBRARY (spwsdl5 ${LIBRARY_TYPE}
    WSBasicTypes.cpp
    WSComplexType.cpp
    WSListener.cpp
    WSParser.cpp
    WSParserComplexType.cpp
    WSRequest.cpp
    WSRestriction.cpp
    WSTypeTranslator.cpp
    protocol/WSStaticHttpProtocol.cpp
    protocol/WSWebServiceProtocol.cpp
    protocol/WSWebSocketsProtocol.cpp
)

TARGET_LINK_LIBRARIES(spwsdl5 sputil5)

INSTALL(TARGETS spwsdl5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
