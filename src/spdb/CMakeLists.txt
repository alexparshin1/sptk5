# SPDB library

IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

ADD_LIBRARY (spdb5 ${LIBRARY_TYPE}
    CDatabaseField.cpp CParamBinding.cpp CParam.cpp CParamList.cpp
    CQuery.cpp CTransaction.cpp CDatabaseConnectionString.cpp
    CDatabaseConnection.cpp CDatabaseConnectionPool.cpp)

TARGET_LINK_LIBRARIES (spdb5 sputil5)

INSTALL(TARGETS spdb5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
