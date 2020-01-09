ADD_CUSTOM_COMMAND(
    OUTPUT  Service/TestServiceBase.cpp
    DEPENDS Test.wsdl
    COMMAND wsdl2cxx
    ARGS Test.wsdl Service
)

ADD_SUBDIRECTORY (Service)
