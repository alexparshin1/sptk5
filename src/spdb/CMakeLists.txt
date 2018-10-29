# SPDB library

IF (LIBRARY_TYPE STREQUAL "SHARED")
  ADD_DEFINITIONS (-DSP_LIBRARY)
ENDIF(LIBRARY_TYPE STREQUAL "SHARED")

ADD_LIBRARY (spdb5 ${LIBRARY_TYPE}
    AutoDatabaseConnection.cpp
    DatabaseField.cpp QueryParameterBinding.cpp QueryParameter.cpp QueryParameterList.cpp
    Query.cpp Transaction.cpp DatabaseConnectionString.cpp
        PoolDatabaseConnection.cpp DatabaseConnectionPool.cpp DatabaseTests.cpp)

SET_TARGET_PROPERTIES(spdb5 PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

TARGET_LINK_LIBRARIES (spdb5 sputil5)

INSTALL(TARGETS spdb5 RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
