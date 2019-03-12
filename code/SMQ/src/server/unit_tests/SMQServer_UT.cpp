#if USE_GTEST

static const MQProtocolType protocolType = MP_SMQ;
static const size_t messageCount {100};

TEST(SPTK_SMQServer, minimal)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4001};
    Host            serverHost("localhost", serverPort);

    SMQServer smqServer(MP_SMQ, "user", "secret", logEngine);

    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient smqSender(protocolType, "test-sender");
    ASSERT_NO_THROW(smqSender.connect(serverHost, "user", "secret", false, connectTimeout));

    SMQClient smqReceiver(protocolType, "test-receiver");
    ASSERT_NO_THROW(smqReceiver.connect(serverHost, "user", "secret", false, connectTimeout));
    ASSERT_NO_THROW(smqReceiver.subscribe("test-queue", std::chrono::milliseconds()));
    this_thread::sleep_for(milliseconds(10)); // Wait until subscription is completed

    auto testMessage = make_shared<Message>(Message::MESSAGE, Buffer("This is SMQ test"));
    for (size_t m = 0; m < messageCount; m++)
        smqSender.send("test-queue", testMessage, sendTimeout);

    size_t maxWait = 1000;
    while (smqReceiver.hasMessages() < messageCount) {
        this_thread::sleep_for(milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount, smqReceiver.hasMessages());

    for (size_t m = 0; m < messageCount; m++) {
        auto msg = smqReceiver.getMessage(milliseconds(100));
        if (msg) {
            EXPECT_STREQ("test-queue", msg->destination().c_str());
            EXPECT_STREQ("This is SMQ test", msg->c_str());
        }
    }

    smqSender.disconnect(true);
    smqReceiver.disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, shortMessages)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4002};
    Host            serverHost("localhost", serverPort);

    SMQServer smqServer(MP_SMQ, "user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1); // Wait until subscription is completed

    SMQClient smqSender(protocolType, "test-client1");
    ASSERT_NO_THROW(smqSender.connect(serverHost, "user", "secret", false, connectTimeout));

    SMQClient smqReceiver(protocolType, "test-client1");
    ASSERT_NO_THROW(smqReceiver.connect(serverHost, "user", "secret", false, connectTimeout));
    smqReceiver.subscribe("test-queue", std::chrono::milliseconds());
    this_thread::sleep_for(milliseconds(10));

    auto msg = make_shared<Message>(Message::MESSAGE, Buffer(""));
    for (size_t m = 0; m < messageCount; m++) {
        msg->headers()["subject"] = "subject " + to_string(m);
        msg->set("data " + to_string(m));
        smqSender.send("test-queue", msg, sendTimeout);
    }

    size_t maxWait = 1000;
    while (smqReceiver.hasMessages() < messageCount) {
        this_thread::sleep_for(milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount, smqReceiver.hasMessages());

    for (size_t m = 0; m < smqReceiver.hasMessages(); m++) {
        auto message = smqReceiver.getMessage(milliseconds(100));
        EXPECT_STREQ((*message)["subject"].c_str(), ("subject " + to_string(m)).c_str());
        EXPECT_STREQ(message->c_str(), ("data " + to_string(m)).c_str());
    }

    smqSender.disconnect(true);
    smqReceiver.disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, multiClients)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4003};
    Host            serverHost("localhost", serverPort);

    SMQServer smqServer(protocolType, "user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient sender(protocolType, "sender");
    ASSERT_NO_THROW(sender.connect(serverHost, "user", "secret", false, connectTimeout));

    size_t clientCount = 3;
    vector< shared_ptr<SMQClient> > receivers;
    size_t clientIndex = 0;
    for (size_t index = 0; index < clientCount; index++) {
        String clientId = "receiver" + to_string(clientIndex);
        auto client = make_shared<SMQClient>(protocolType, clientId);
        receivers.push_back(client);
        String clientQueue = "test-queue" + to_string(clientIndex);
        ASSERT_NO_THROW(client->connect(serverHost, "user", "secret", false, connectTimeout));
        ASSERT_NO_THROW(client->subscribe(clientQueue, sendTimeout));
        clientIndex++;
    }

    this_thread::sleep_for(milliseconds(100));

    auto msg1 = make_shared<Message>();
    for (size_t m = 0; m < messageCount; m++) {
        msg1->headers()["subject"] = "subject " + to_string(m);
        msg1->set("data " + to_string(m));
        for (size_t clientIndex1 = 0; clientIndex1 < clientCount; clientIndex1++) {
            String destination("test-queue" + to_string(clientIndex1));
            sender.send(destination, msg1, sendTimeout);
        }
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * clientCount) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = 0;
        for (auto& client: receivers) {
            totalMessages += client->hasMessages();
        }
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * clientCount, totalMessages);

    for (auto& client: receivers) {
        for (size_t m = 0; m < messageCount; m++) {
            auto msg = client->getMessage(milliseconds(1000));
            if (msg) {
                EXPECT_STREQ((*msg)["subject"].c_str(), ("subject " + to_string(m)).c_str());
                EXPECT_STREQ(msg->c_str(), ("data " + to_string(m)).c_str());
            } else
                FAIL() << "Received " << m << " messages out of " << messageCount;
        }
    }

    for (auto& client: receivers)
        client->disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, singleClientMultipleQueues)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4004};
    Host            serverHost("localhost", serverPort);

    SMQServer smqServer(MP_SMQ, "user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient sender(protocolType, "sender");
    ASSERT_NO_THROW(sender.connect(serverHost, "user", "secret", false, connectTimeout));

    SMQClient client(protocolType, "receiver");
    ASSERT_NO_THROW(client.connect(serverHost, "user", "secret", false, connectTimeout));

    vector<String> queueNames = { "test-queue1",  "test-queue2", "test-queue3", };
    for (auto& queueName: queueNames)
        ASSERT_NO_THROW(client.subscribe(queueName, std::chrono::milliseconds()));

    this_thread::sleep_for(milliseconds(100));

    for (auto& queueName: queueNames) {
        for (size_t m = 0; m < messageCount; m++) {
            auto msg1 = make_shared<Message>();
            msg1->headers()["subject"] = "subject " + to_string(m);
            msg1->set("data " + to_string(m));
            sender.send(queueName, msg1, sendTimeout);
        }
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * queueNames.size()) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = client.hasMessages();
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * queueNames.size(), totalMessages);

    COUT("Client " << client.getClientId() << " has " <<client.hasMessages() << " messages" << endl);

    sender.disconnect(true);
    client.disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, multipleClientsSingleQueue)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4005};
    Host            serverHost("localhost", serverPort);

    SMQServer smqServer(MP_SMQ, "user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient sender(protocolType, "sender");
    ASSERT_NO_THROW(sender.connect(serverHost, "user", "secret", false, connectTimeout));

    size_t clientCount = 3;
    vector<shared_ptr<SMQClient>> receivers;
    size_t clientIndex = 0;
    for (size_t index = 0; index < clientCount; index++) {
        String clientId = "receiver" + to_string(clientIndex);
        auto client = make_shared<SMQClient>(protocolType, clientId);
        receivers.push_back(client);
        ASSERT_NO_THROW(client->connect(serverHost, "user", "secret", false, connectTimeout));
        ASSERT_NO_THROW(client->subscribe("/queue/test", std::chrono::milliseconds()));
        clientIndex++;
    }

    this_thread::sleep_for(milliseconds(100));

    auto msg1 = make_shared<Message>();
    for (size_t m = 0; m < messageCount; m++) {
        msg1->headers()["subject"] = "subject " + to_string(m);
        msg1->set("data " + to_string(m));
        for (size_t clientIndex1 = 0; clientIndex1 < clientCount; clientIndex1++)
            sender.send("/queue/test", msg1, sendTimeout);
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * clientCount) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = 0;
        for (auto& client: receivers) {
            totalMessages += client->hasMessages();
        }
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * clientCount, totalMessages);

    for (auto& client: receivers)
    COUT("Client " << client->getClientId() << " has " <<client->hasMessages() << " messages" << endl);

    for (auto& client: receivers)
        client->disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, multipleClientsSingleTopic)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4006};
    Host            serverHost("localhost", serverPort);

    SMQServer smqServer(MP_SMQ, "user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient sender(protocolType, "sender");
    ASSERT_NO_THROW(sender.connect(serverHost, "user", "secret", false, connectTimeout));

    size_t clientCount = 10;
    vector<shared_ptr<SMQClient>> receivers;
    size_t clientIndex = 0;
    for (size_t index = 0; index < clientCount; index++) {
        String clientId = "receiver" + to_string(clientIndex);
        auto client = make_shared<SMQClient>(protocolType, clientId);
        receivers.push_back(client);
        ASSERT_NO_THROW(client->connect(serverHost, "user", "secret", false, connectTimeout));
        ASSERT_NO_THROW(client->subscribe("/topic/test", std::chrono::milliseconds()));
        clientIndex++;
    }

    this_thread::sleep_for(milliseconds(100));

    auto msg1 = make_shared<Message>();
    for (size_t m = 0; m < messageCount; m++) {
        msg1->headers()["subject"] = "subject " + to_string(m);
        msg1->set("data " + to_string(m));
        sender.send("/topic/test", msg1, sendTimeout);
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * clientCount) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = 0;
        for (auto& client: receivers)
            totalMessages += client->hasMessages();
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * clientCount, totalMessages);

    for (auto& client: receivers)
    COUT("Client " << client->getClientId() << " has " << client->hasMessages() << " messages" << endl);

    for (auto& client: receivers)
        client->disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, mqttMinimal)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4007};
    Host            serverHost("localhost", serverPort);
    MQProtocolType  protocolType(MP_MQTT);

    SMQServer smqServer(protocolType, "user", "secret", logEngine);

    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient smqReceiver(protocolType, "test-receiver");
    ASSERT_NO_THROW(smqReceiver.connect(serverHost, "user", "secret", false, connectTimeout));
    ASSERT_NO_THROW(smqReceiver.subscribe("test-queue", std::chrono::milliseconds()));
    this_thread::sleep_for(milliseconds(10)); // Wait until subscription is completed

    SMQClient smqSender(protocolType, "test-sender");
    ASSERT_NO_THROW(smqSender.connect(serverHost, "user", "secret", false, connectTimeout));

    auto testMessage = make_shared<Message>(Message::MESSAGE, Buffer("This is SMQ test"));
    for (size_t m = 0; m < messageCount; m++)
        smqSender.send("test-queue", testMessage, sendTimeout);

    size_t maxWait = 1000;
    while (smqReceiver.hasMessages() < messageCount) {
        this_thread::sleep_for(milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount, smqReceiver.hasMessages());

    for (size_t m = 0; m < messageCount; m++) {
        auto msg = smqReceiver.getMessage(milliseconds(100));
        if (msg) {
            EXPECT_STREQ("test-queue", msg->destination().c_str());
            EXPECT_STREQ("This is SMQ test", msg->c_str());
        }
    }

    smqSender.disconnect(true);
    smqReceiver.disconnect(true);

    smqServer.stop();
}

TEST(SPTK_SMQServer, mqttLastWill)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    uint16_t        serverPort {4007};
    Host            serverHost("localhost", serverPort);
    MQProtocolType  protocolType(MP_MQTT);

    SMQServer smqServer(protocolType, "user", "secret", logEngine);

    ASSERT_NO_THROW(smqServer.listen(serverPort));

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient smqReceiver1(protocolType, "test-receiver1");
    SMQClient smqReceiver2(protocolType, "test-receiver2");
    auto lastWillMessage = make_unique<MQLastWillMessage>("last_will", "Client connection terminated");
    SMQClient smqSender(protocolType, "test-sender");
    ASSERT_NO_THROW(smqSender.setLastWillMessage(lastWillMessage));
    ASSERT_NO_THROW(smqSender.connect(serverHost, "user", "secret", false, connectTimeout));

    smqReceiver1.connect(serverHost, "user", "secret", false, connectTimeout);
    smqReceiver2.connect(serverHost, "user", "secret", false, connectTimeout);
    smqReceiver1.subscribe("last_will", std::chrono::milliseconds());
    smqReceiver2.subscribe("last_will", std::chrono::milliseconds());

    this_thread::sleep_for(milliseconds(100)); // Wait until subscription is completed

    smqSender.disconnect(true);

    int counter = 0;
    while (true) {
        auto msg = smqReceiver1.getMessage(milliseconds(50));
        if (msg) {
            counter++;
            EXPECT_STREQ("Client connection terminated", msg->c_str());
        } else
            break;
    }
    EXPECT_EQ(counter, 1);

    counter = 0;
    while (true) {
        auto msg = smqReceiver2.getMessage(milliseconds(50));
        if (msg) {
            counter++;
            EXPECT_STREQ("Client connection terminated", msg->c_str());
        } else
            break;
    }
    EXPECT_EQ(counter, 1);

    smqReceiver1.disconnect(true);
    smqReceiver2.disconnect(true);
    smqServer.stop();
}

#endif
