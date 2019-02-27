/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.cpp - description                            ║
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

using namespace std;
using namespace sptk;
using namespace chrono;

SMQServer::SMQServer(MQProtocolType protocol, const String& username, const String& password, LogEngine& logEngine)
: TCPServer("SMQServer", 16, &logEngine),
  m_protocol(protocol),
  m_username(username), m_password(password),
  m_socketEvents("SMQ Server", SMQServer::socketEventCallback, milliseconds(100))
{
}

SMQServer::~SMQServer()
{
    clear();
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

void SMQServer::closeConnection(ServerConnection* connection)
{
    auto* smqConnection = dynamic_cast<SMQConnection*>(connection);
    if (smqConnection != nullptr) {
        String clientId = smqConnection->clientId();
        lock_guard<mutex> lock(m_mutex);
        m_clientIds.erase(clientId);
        m_connections.erase(smqConnection);
    }
}

void SMQServer::socketEventCallback(void *userData, SocketEventType eventType)
{
    auto* connection = (SMQConnection*) userData;
    auto* smqServer = dynamic_cast<SMQServer*>(&connection->server());

    if (eventType == ET_CONNECTION_CLOSED) {
        smqServer->closeConnection(connection);
        delete connection;
        return;
    }

    try {
        while (connection != nullptr && connection->socket().socketBytes() > 0) {

            SMessage msg;
            MQProtocol& protocol = connection->protocol();
            protocol.readMessage(msg);

            switch (msg->type()) {
                case Message::CONNECT:
                    if (!smqServer->authenticate((*msg)["client_id"], (*msg)["username"], (*msg)["password"])) {
                        smqServer->closeConnection(connection);
                        delete connection;
                        connection = nullptr;
                    } else
                        connection->setupClient((*msg)["client_id"]);
                    protocol.ack(Message::CONNECT, "");
                    break;
                case Message::SUBSCRIBE:
                    smqServer->subscribe(connection, Strings(msg->destination(), ","));
                    break;
                case Message::UNSUBSCRIBE:
                    smqServer->unsubscribe(connection, msg->destination());
                    break;
                case Message::MESSAGE:
                    smqServer->distributeMessage(msg);
                    break;
                case Message::DISCONNECT:
                    smqServer->closeConnection(connection);
                    delete connection;
                    connection = nullptr;
                    break;
                default:
                    break;
            }
        }
    }
    catch (const Exception& e) {
        if (connection != nullptr) {
            smqServer->closeConnection(connection);
            delete connection;
        }
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

void SMQServer::subscribe(SMQConnection* connection, const Strings& destinations)
{
    m_subscriptions.subscribe(connection, destinations);
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

void SMQServer::execute(Runable*)
{
    // SMQServer doesn't use tasks model
}

MQProtocolType SMQServer::protocol() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_protocol;
}

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
    this_thread::sleep_for(milliseconds(1));

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

    COUT("Client " << client.clientId() << " has " <<client.hasMessages() << " messages" << endl);

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
        ASSERT_NO_THROW(client->subscribe("test-queue", std::chrono::milliseconds()));
        clientIndex++;
    }

    this_thread::sleep_for(milliseconds(100));

    auto msg1 = make_shared<Message>();
    for (size_t m = 0; m < messageCount; m++) {
        msg1->headers()["subject"] = "subject " + to_string(m);
        msg1->set("data " + to_string(m));
        for (size_t clientIndex1 = 0; clientIndex1 < clientCount; clientIndex1++)
            sender.send("test-queue", msg1, sendTimeout);
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
        COUT("Client " << client->clientId() << " has " <<client->hasMessages() << " messages" << endl);

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
        COUT("Client " << client->clientId() << " has " << client->hasMessages() << " messages" << endl);

    for (auto& client: receivers)
        client->disconnect(true);

    smqServer.stop();
}

#endif
