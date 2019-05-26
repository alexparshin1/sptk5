INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(smq ${LIBRARY_TYPE}
    src/clients/Message.cpp src/clients/BaseMQClient.cpp src/clients/TCPMQClient.cpp src/clients/SMQClient.cpp
    src/protocols/MQProtocol.cpp src/protocols/MQTTFrame.cpp src/protocols/MQTTProtocol.cpp src/protocols/SMQProtocol.cpp
    src/server/SMQConnection.cpp
    src/server/SMQPersistentSendQueue.cpp
    src/server/SMQServer.cpp
    src/server/SMQSubscription.cpp src/server/SMQSubscriptions.cpp
    src/storage/SMQStorage.cpp
    src/unit_tests/SMQServer_UT.cpp
    src/server/SMQSendQueue.cpp
    src/server/SMQSendThreadPool.cpp
    src/persistent/Handle.cpp src/persistent/ListPool.cpp src/persistent/MemoryMappedFile.cpp
    src/persistent/MemoryBucket.cpp src/persistent/MemoryPool.cpp
    src/persistent/PersistentList.cpp src/persistent/PersistentString.cpp
)

TARGET_LINK_LIBRARIES(smq sputil5)

ADD_EXECUTABLE (smq_server src/server/smq_server)

TARGET_LINK_LIBRARIES(smq_server smq)

INSTALL(TARGETS smq_server smq
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

INSTALL (DIRECTORY "smq"
      DESTINATION include
      DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE )
