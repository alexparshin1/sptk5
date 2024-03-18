IF (UNIX)

ADD_CUSTOM_COMMAND(
    OUTPUT  Service/TestServiceBase.cpp
    DEPENDS Test.wsdl wsdl2cxx
    COMMAND wsdl2cxx
    ARGS Test.wsdl -d Service
)

ADD_SUBDIRECTORY(Service)
ADD_LIBRARY(WebServiceUnitTests TestWebService.cpp)
TARGET_LINK_LIBRARIES(WebServiceUnitTests TestWebService spwsdl5)

INSTALL(TARGETS WebServiceUnitTests 
        RUNTIME DESTINATION bin COMPONENT unit_tests
        LIBRARY DESTINATION lib COMPONENT unit_tests
        ARCHIVE DESTINATION lib COMPONENT unit_tests)

IF (SET_RPATH)
    SET_TARGET_PROPERTIES( WebServiceUnitTests PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" )
ENDIF ()

ENDIF ()
