ADD_CUSTOM_COMMAND(
    OUTPUT  Service/TestServiceBase.cpp
    DEPENDS Test.wsdl
    COMMAND wsdl2cxx
    ARGS Test.wsdl Service
)

ADD_SUBDIRECTORY(Service)
ADD_LIBRARY(WebServiceUnitTests TestWebService.cpp)
TARGET_LINK_LIBRARIES(WebServiceUnitTests TestWebService)
