ADD_EXECUTABLE (load_balance load_balance.cpp LoadBalance.cpp Channel.cpp)
TARGET_LINK_LIBRARIES (load_balance sputil5)

INSTALL(TARGETS load_balance
    RUNTIME DESTINATION bin COMPONENT core
    LIBRARY DESTINATION lib COMPONENT core
    ARCHIVE DESTINATION lib COMPONENT core
  )


