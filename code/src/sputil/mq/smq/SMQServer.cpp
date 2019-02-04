/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/mq/SMQServer.h>
#include <sptk5/mq/SMQClient.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;
using namespace chrono;

SMQServer::SMQServer(const String& username, const String& password, LogEngine& logEngine)
: TCPServer("SMQServer", 16, &logEngine),
  m_username(username), m_password(password),
  m_socketEvents(SMQServer::socketEventCallback, seconds(1))
{
}

void SMQServer::stop()
{
    m_socketEvents.terminate();
	TCPServer::stop();
    m_socketEvents.stop();
    log(LP_NOTICE, "Server stopped");
}

ServerConnection* SMQServer::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    auto* newConnection = new SMQConnection(*this, connectionSocket, peer);

    lock_guard<mutex> lock(m_mutex);
    m_connections.insert(newConnection);

    return newConnection;
}

void SMQServer::removeConnection(ServerConnection* connection)
{
    auto* smqConnection = dynamic_cast<SMQConnection*>(connection);
    String clientId = smqConnection->getClientId();

    delete smqConnection;

    lock_guard<mutex> lock(m_mutex);
    m_clientIds.erase(clientId);
    m_connections.erase(smqConnection);
}

void SMQServer::socketEventCallback(void *userData, SocketEventType eventType)
{
    SMQConnection* connection = (SMQConnection*) userData;

    if (eventType == ET_CONNECTION_CLOSED) {
        connection->terminate();
        return;
    }

    SMQServer* smqServer = dynamic_cast<SMQServer*>(&connection->server());

    try {
        while (connection->socket().socketBytes() > 0) {

            auto msg = SMQMessage::readRawMessage(connection->socket());

            switch (msg->type()) {
                case Message::CONNECT:
                    if (!smqServer->authenticate((*msg)["clientid"], (*msg)["username"], (*msg)["password"]))
                        smqServer->removeConnection(connection);
                    else
                        connection->setClientId((*msg)["clientid"]);
                    break;
                case Message::SUBSCRIBE:
                    smqServer->subscribe(connection, msg->destination());
                    break;
                case Message::UNSUBSCRIBE:
                    smqServer->unsubscribe(connection, msg->destination());
                    break;
                case Message::MESSAGE:
                    smqServer->distributeMessage(msg);
                    break;
                case Message::DISCONNECT:
                    smqServer->removeConnection(connection);
                    break;
                default:
                    break;
            }
        }
    }
    catch (const Exception& e) {
        smqServer->removeConnection(connection);
        connection->terminate();
        smqServer->log(LP_ERROR, e.message());
    }
}

void SMQServer::distributeMessage(SMessage message)
{
    m_subscriptions.deliverMessage(message->destination(), message);
}

bool SMQServer::authenticate(const String& clientId, const String& username, const String& password)
{
    lock_guard<mutex> lock(m_mutex);

    String logPrefix("(" + clientId + ") ");

    if (m_clientIds.find(clientId) != m_clientIds.end()) {
        log(LP_ERROR, logPrefix + "Duplicate client id");
        return false;
    }

    if (username != m_username || password != m_password) {
        log(LP_ERROR, logPrefix + "Invalid username or password");
        return false;
    }

    log(LP_INFO, logPrefix + "Authenticated");

    return true;
}

void SMQServer::watchSocket(TCPSocket& socket, void* userData)
{
    m_socketEvents.add(socket, userData);
}

void SMQServer::forgetSocket(TCPSocket& socket)
{
    m_socketEvents.remove(socket);
}

void SMQServer::run()
{
    Thread::run();
    log(LP_NOTICE, "Server started");
}

void SMQServer::subscribe(SMQConnection* connection, const String& destination)
{
    m_subscriptions.subscribe(connection, destination);
}

void SMQServer::unsubscribe(SMQConnection* connection, const String& destination)
{
    m_subscriptions.unsubscribe(connection, destination);
}

void SMQServer::clear()
{
    m_subscriptions.clear();
    lock_guard<mutex> lock(m_mutex);
    for (auto* connection: m_connections)
        delete connection;
    m_connections.clear();
    m_clientIds.clear();
}

SMQServer::~SMQServer()
{
    clear();
}

#if USE_GTEST

static size_t messageCount {100};

TEST(SPTK_SMQServer, minimal)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient smqSender;
    ASSERT_NO_THROW(smqSender.connect(Host("localhost:4000"), "test-sender", "user", "secret"));

    SMQClient smqReceiver;
    ASSERT_NO_THROW(smqReceiver.connect(Host("localhost:4000"), "test-receiver", "user", "secret"));
    ASSERT_NO_THROW(smqReceiver.subscribe("test-queue"));

    for (size_t m = 0; m < messageCount; m++)
        smqSender.sendMessage(Message(Message::MESSAGE, Buffer("This is SMQ test"), "test-queue"));

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

    smqSender.terminate();
    smqReceiver.terminate();

    smqSender.disconnect();
    smqReceiver.disconnect();

    smqServer.stop();
}

TEST(SPTK_SMQServer, shortMessages)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient smqSender;
    ASSERT_NO_THROW(smqSender.connect(Host("localhost:4000"), "test-client1", "user", "secret"));

    SMQClient smqReceiver;
    ASSERT_NO_THROW(smqReceiver.connect(Host("localhost:4000"), "test-client1", "user", "secret"));
    smqReceiver.subscribe("test-queue");

    Message msg(Message::MESSAGE, Buffer(""), "test-queue");
    for (size_t m = 0; m < messageCount; m++) {
        msg["subject"] = "subject " + to_string(m);
        msg.set("data " + to_string(m));
        smqSender.sendMessage(msg);
    }

    size_t maxWait = 1000;
    while (smqReceiver.hasMessages() < messageCount) {
        this_thread::sleep_for(milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount, smqReceiver.hasMessages());

    for (size_t m = 0; m < messageCount; m++) {
        auto message = smqReceiver.getMessage(milliseconds(100));
        EXPECT_STREQ((*message)["subject"].c_str(), ("subject " + to_string(m)).c_str());
        EXPECT_STREQ(message->c_str(), ("data " + to_string(m)).c_str());
    }

    smqSender.terminate();
    smqReceiver.terminate();

    smqSender.disconnect();
    smqReceiver.disconnect();

    smqServer.stop();
}

TEST(SPTK_SMQServer, multiClients)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    Host            serverHost("localhost:4000");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient sender;
    ASSERT_NO_THROW(sender.connect(serverHost, "sender", "user", "secret"));

    size_t clientCount = 3;
    vector<SMQClient> receivers(clientCount);
    size_t clientIndex = 0;
    for (auto& client: receivers) {
        String clientId = "receiver" + to_string(clientIndex);
        String clientQueue = "test-queue" + to_string(clientIndex);
        ASSERT_NO_THROW(client.connect(serverHost, clientId, "user", "secret"));
        ASSERT_NO_THROW(client.subscribe(clientQueue));
        clientIndex++;
    }

    Message msg1;
    for (size_t m = 0; m < messageCount; m++) {
        msg1["subject"] = "subject " + to_string(m);
        msg1.set("data " + to_string(m));
        for (size_t clientIndex1 = 0; clientIndex1 < clientCount; clientIndex1++) {
            msg1.destination("test-queue" + to_string(clientIndex1));
            sender.sendMessage(msg1);
        }
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * clientCount) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = 0;
        for (auto& client: receivers) {
            totalMessages += client.hasMessages();
        }
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * clientCount, totalMessages);

    for (auto& client: receivers) {
        for (size_t m = 0; m < messageCount; m++) {
            auto msg = client.getMessage(milliseconds(100));
            EXPECT_STREQ((*msg)["subject"].c_str(), ("subject " + to_string(m)).c_str());
            EXPECT_STREQ(msg->c_str(), ("data " + to_string(m)).c_str());
        }
    }

    for (auto& client: receivers)
        client.terminate();

    for (auto& client: receivers)
        client.disconnect();

    smqServer.stop();
}

TEST(SPTK_SMQServer, singleClientMultipleQueues)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    Host            serverHost("localhost:4000");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient sender;
    ASSERT_NO_THROW(sender.connect(serverHost, "sender", "user", "secret"));

    SMQClient client;
    ASSERT_NO_THROW(client.connect(serverHost, "receiver", "user", "secret"));

    vector<String> queueNames = { "test-queue1",  "test-queue2", "test-queue3", };
    for (auto& queueName: queueNames)
        ASSERT_NO_THROW(client.subscribe(queueName));

    this_thread::sleep_for(milliseconds(100));

    for (auto& queueName: queueNames) {
        for (size_t m = 0; m < messageCount; m++) {
            Message msg1;
            msg1["subject"] = "subject " + to_string(m);
            msg1.set("data " + to_string(m));
            msg1.destination(queueName);
            sender.sendMessage(msg1);
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

    COUT("Client " << client.clientId() << " has " <<client.hasMessages() << " messages" << endl);

    sender.terminate();
    client.terminate();

    sender.disconnect();
    client.disconnect();

    smqServer.stop();
}

TEST(SPTK_SMQServer, multipleClientsSingleQueue)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");
    Host            serverHost("localhost:4000");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient sender;
    ASSERT_NO_THROW(sender.connect(serverHost, "sender", "user", "secret"));

    size_t clientCount = 3;
    vector<SMQClient> receivers(clientCount);
    size_t clientIndex = 0;
    for (auto& client: receivers) {
        String clientId = "receiver" + to_string(clientIndex);
        ASSERT_NO_THROW(client.connect(serverHost, clientId, "user", "secret"));
        ASSERT_NO_THROW(client.subscribe("test-queue"));
        clientIndex++;
    }

    this_thread::sleep_for(milliseconds(100));

    Message msg1;
    for (size_t m = 0; m < messageCount; m++) {
        msg1["subject"] = "subject " + to_string(m);
        msg1.set("data " + to_string(m));
        for (size_t clientIndex1 = 0; clientIndex1 < clientCount; clientIndex1++) {
            msg1.destination("test-queue");
            sender.sendMessage(msg1);
        }
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * clientCount) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = 0;
        for (auto& client: receivers) {
            totalMessages += client.hasMessages();
        }
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * clientCount, totalMessages);

    for (auto& client: receivers)
    COUT("Client " << client.clientId() << " has " <<client.hasMessages() << " messages" << endl);

    for (auto& client: receivers)
        client.terminate();

    for (auto& client: receivers)
        client.disconnect();

    smqServer.stop();
}

TEST(SPTK_SMQServer, multipleClientsSingleTopic)
{
    Buffer buffer;
    FileLogEngine logEngine("SMQServer.log");
    Host serverHost("localhost:4000");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient sender;
    ASSERT_NO_THROW(sender.connect(serverHost, "sender", "user", "secret"));

    size_t clientCount = 10;
    vector<SMQClient> receivers(clientCount);
    size_t clientIndex = 0;
    for (auto& client: receivers) {
        String clientId = "receiver" + to_string(clientIndex);
        ASSERT_NO_THROW(client.connect(serverHost, clientId, "user", "secret"));
        ASSERT_NO_THROW(client.subscribe("/topic/test"));
        clientIndex++;
    }

    this_thread::sleep_for(milliseconds(100));

    Message msg1;
    for (size_t m = 0; m < messageCount; m++) {
        msg1["subject"] = "subject " + to_string(m);
        msg1.set("data " + to_string(m));
        msg1.destination("/topic/test");
        sender.sendMessage(msg1);
    }

    size_t totalMessages = 0;
    size_t maxWait = 1000;
    while (totalMessages < messageCount * clientCount) {
        this_thread::sleep_for(milliseconds(1));
        totalMessages = 0;
        for (auto& client: receivers) {
            totalMessages += client.hasMessages();
        }
        maxWait--;
        if (maxWait == 0)
            break;
    }

    EXPECT_EQ(messageCount * clientCount, totalMessages);

    for (auto& client: receivers)
        COUT("Client " << client.clientId() << " has " << client.hasMessages() << " messages" << endl);

    for (auto& client: receivers)
        client.terminate();

    for (auto& client: receivers)
        client.disconnect();

    smqServer.stop();
}

#endif
