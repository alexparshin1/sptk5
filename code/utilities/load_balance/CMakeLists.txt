ADD_EXECUTABLE (load_balance load_balance.cpp LoadBalance.cpp Channel.cpp)
TARGET_LINK_LIBRARIES (load_balance sputil5)

INSTALL(TARGETS load_balance
    RUNTIME DESTINATION bin COMPONENT sputil5
    LIBRARY DESTINATION lib COMPONENT sputil5
    ARCHIVE DESTINATION lib COMPONENT sputil5
  )


