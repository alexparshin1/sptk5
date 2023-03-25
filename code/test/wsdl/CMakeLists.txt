IF (UNIX)

ADD_CUSTOM_COMMAND(
    OUTPUT  Service/TestServiceBase.cpp
    DEPENDS Test.wsdl wsdl2cxx
    COMMAND wsdl2cxx
    ARGS Test.wsdl Service
)

ADD_SUBDIRECTORY(Service)
ADD_LIBRARY(WebServiceUnitTests TestWebService.cpp)
TARGET_LINK_LIBRARIES(WebServiceUnitTests TestWebService spwsdl5)
INSTALL(TARGETS WebServiceUnitTests RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

IF (SET_RPATH)
    SET_TARGET_PROPERTIES( WebServiceUnitTests PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" )
ENDIF ()

ENDIF ()
