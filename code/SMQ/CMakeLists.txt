ADD_LIBRARY(smq_lib 
    mq/Message.cpp mq/BaseMQClient.cpp mq/TCPMQClient.cpp mq/SMQClient.cpp
    protocols/MQProtocol.cpp protocols/MQTTFrame.cpp protocols/MQTTProtocol.cpp protocols/SMQProtocol.cpp
)

ADD_EXECUTABLE (smq_server
    smq/smq_server
    smq/SMQConnection.cpp smq/SMQServer.cpp
    smq/SMQSubscription.cpp smq/SMQSubscriptions.cpp
)

TARGET_LINK_LIBRARIES(smq_server smq_lib sputil5)

INSTALL(TARGETS smq_server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
