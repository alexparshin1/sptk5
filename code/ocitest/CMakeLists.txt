ADD_EXECUTABLE(ocitest ocitest.cpp)
TARGET_LINK_LIBRARIES(ocitest ${OCI_LIBRARY} sputil5 spdb5 spdb5_oracle_oci)
