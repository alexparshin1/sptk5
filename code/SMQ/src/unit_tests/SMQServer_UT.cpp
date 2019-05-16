/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer_UT.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <smq/server/SMQServer.h>
#include <smq/clients/SMQClient.h>
#include <smq/protocols/MQTTProtocol.h>
#include <smq/unit_tests/SMQServer_UT.h>

using namespace std;
using namespace sptk;
using namespace chrono;
using namespace smq;

SMQServer_UT::SMQServer_UT()
{
}

#if USE_GTEST

shared_ptr<FileLogEngine> logEngine;

unique_ptr<SMQServer> createSMQServer(const MQProtocolType protocolType, const Host& serverHost)
{
    if (!logEngine)
        logEngine = make_shared<FileLogEngine>("SMQServer.log");

    unique_ptr<SMQServer> smqServer = make_unique<SMQServer>(protocolType, "user", "secret", *logEngine);
    smqServer->listen(serverHost.port());

    return smqServer;
}

TEST(SPTK_SMQServer, minimal)
{
    Buffer          buffer;

    size_t          messageCount {100};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4001);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    smqServer->stop();
}

TEST(SPTK_SMQServer, shortMessages)
{
    Buffer          buffer;

    size_t          messageCount {10};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4002);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    map<String,String> receivedMessages;
    for (size_t m = 0; m < messageCount; m++) {
        auto message = smqReceiver.getMessage(milliseconds(100));
        receivedMessages[ (*message)["subject"] ] = message->c_str();
    }

    for (size_t m = 0; m < messageCount; m++) {
        String data = receivedMessages[ "subject " + to_string(m) ];
        EXPECT_STREQ(data.c_str(), ("data " + to_string(m)).c_str());
    }

    smqSender.disconnect(true);
    smqReceiver.disconnect(true);

    smqServer->stop();
}

TEST(SPTK_SMQServer, multiClients)
{
    Buffer          buffer;

    size_t          messageCount {100};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4003);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    smqServer->stop();
}

TEST(SPTK_SMQServer, singleClientMultipleQueues)
{
    Buffer          buffer;

    size_t          messageCount {100};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4004);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    smqServer->stop();
}

TEST(SPTK_SMQServer, multipleClientsSingleQueue)
{
    Buffer          buffer;

    size_t          messageCount {100};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4005);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    smqServer->stop();
}

TEST(SPTK_SMQServer, multipleClientsSingleTopic)
{
    Buffer          buffer;

    size_t          messageCount {100};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4006);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    smqServer->stop();
}

TEST(SPTK_SMQServer, mqttMinimal)
{
    Buffer          buffer;

    size_t          messageCount {100};
    MQProtocolType  protocolType {MP_MQTT};
    Host            serverHost("localhost", 4007);

    auto smqServer = createSMQServer(protocolType, serverHost);

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

    smqServer->stop();
}

TEST(SPTK_SMQServer, mqttLastWill)
{
    Buffer          buffer;

    MQProtocolType  protocolType {MP_MQTT};
    Host            serverHost("localhost", 4008);

    auto smqServer = createSMQServer(protocolType, serverHost);

    seconds connectTimeout(10);

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
    smqServer->stop();
}

TEST(SPTK_SMQServer, performanceSingleSenderSingleReceiver)
{
    Buffer          buffer;

    size_t          messageCount {100000};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4009);

    auto smqServer = createSMQServer(protocolType, serverHost);

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    SMQClient smqSender(protocolType, "test-sender");
    ASSERT_NO_THROW(smqSender.connect(serverHost, "user", "secret", false, connectTimeout));

    SMQClient smqReceiver(protocolType, "test-receiver");
    ASSERT_NO_THROW(smqReceiver.connect(serverHost, "user", "secret", false, connectTimeout));
    ASSERT_NO_THROW(smqReceiver.subscribe("test-performance", std::chrono::milliseconds()));
    this_thread::sleep_for(milliseconds(10)); // Wait until subscription is completed

    DateTime started("now");

    auto testMessage = make_shared<Message>(Message::MESSAGE, Buffer("This is SMQ performance test"));
    for (size_t m = 0; m < messageCount; m++)
        smqSender.send("test-performance", testMessage, sendTimeout);

    size_t maxWait = 10000;
    while (smqReceiver.hasMessages() < messageCount) {
        this_thread::sleep_for(milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }
    DateTime ended("now");
    milliseconds elapsed = duration_cast<milliseconds>(ended - started);
    double performance = double(messageCount) / elapsed.count();
    COUT("Performance: " << fixed << setprecision(1) << performance << "K msg/s" << endl);

    EXPECT_GT(performance, 10);
    EXPECT_EQ(messageCount, smqReceiver.hasMessages());

    smqSender.disconnect(true);
    smqReceiver.disconnect(true);

    smqServer->stop();
}

TEST(SPTK_SMQServer, performanceMultipleSendersSingleReceiver)
{
    Buffer          buffer;

    size_t          messageCount {100000};
    size_t          senderCount {1000};
    MQProtocolType  protocolType {MP_SMQ};
    Host            serverHost("localhost", 4009);

    auto smqServer = createSMQServer(protocolType, serverHost);

    seconds connectTimeout(10);
    seconds sendTimeout(1);

    vector< shared_ptr<SMQClient> > senders;
    for (size_t i = 0; i < senderCount; i++) {
        auto smqSender = make_shared<SMQClient>(protocolType, "test-sender" + to_string(i));
        ASSERT_NO_THROW(smqSender->connect(serverHost, "user", "secret", false, connectTimeout));
        senders.push_back(smqSender);
    }

    SMQClient smqReceiver(protocolType, "test-receiver");
    ASSERT_NO_THROW(smqReceiver.connect(serverHost, "user", "secret", false, connectTimeout));
    ASSERT_NO_THROW(smqReceiver.subscribe("test-performance", std::chrono::milliseconds()));
    this_thread::sleep_for(milliseconds(10)); // Wait until subscription is completed

    DateTime started("now");

    auto testMessage = make_shared<Message>(Message::MESSAGE, Buffer("This is SMQ performance test"));
    size_t sentMessages = 0;
    while (sentMessages < messageCount) {
        for (auto sender: senders) {
            sender->send("test-performance", testMessage, sendTimeout);
            sentMessages++;
        }
    }

    size_t maxWait = 10000;
    while (smqReceiver.hasMessages() < messageCount) {
        this_thread::sleep_for(milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }
    DateTime ended("now");
    milliseconds elapsed = duration_cast<milliseconds>(ended - started);
    double performance = double(messageCount) / elapsed.count();
    COUT("Performance: " << fixed << setprecision(1) << performance << "K msg/s" << endl);

    EXPECT_GT(performance, 10);
    EXPECT_EQ(messageCount, smqReceiver.hasMessages());

    for (auto sender: senders)
        sender->disconnect(true);
    smqReceiver.disconnect(true);

    smqServer->stop();
}

#endif
