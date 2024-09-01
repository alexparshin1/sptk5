ADD_LIBRARY(spdb5_oracle ${DRIVER_LIBRARY_TYPE}
        OracleOciConnection.cpp
        OracleOciStatement.cpp
        OracleOciParameterBuffer.cpp
        OracleOciDatabaseField.cpp
)

SET_TARGET_PROPERTIES(spdb5_oracle PROPERTIES SOVERSION ${SOVERSION} VERSION ${VERSION})

IF (SET_RPATH)
    SET_TARGET_PROPERTIES(spdb5_oracle PROPERTIES INSTALL_RPATH "")
ENDIF ()

TARGET_LINK_LIBRARIES(spdb5_oracle spdb5 sputil5 ${OCI_LIBRARY})

INSTALL(
        TARGETS spdb5_oracle
        RUNTIME DESTINATION bin COMPONENT db_oracle
        LIBRARY DESTINATION lib COMPONENT db_oracle
        ARCHIVE DESTINATION lib COMPONENT db_oracle)
