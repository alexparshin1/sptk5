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

SMQServer::Connection::Connection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*)
: TCPServerConnection(server, connectionSocket)
{
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server);
    if (smqServer != nullptr)
        smqServer->m_socketEvents.add(socket(), this);
}

SMQServer::Connection::~Connection()
{
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    if (smqServer != nullptr)
        smqServer->m_socketEvents.remove(socket());
}

void SMQServer::Connection::terminate()
{
    socket().close();
    TCPServerConnection::terminate();
}

void SMQServer::Connection::subscribeTo(const String& destination)
{
    lock_guard<mutex> lock(m_mutex);
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    m_subscribedQueue = smqServer->getClientQueue(destination);
}

shared_ptr<SMessageQueue> SMQServer::Connection::subscribedQueue()
{
    lock_guard<mutex> lock(m_mutex);
    return m_subscribedQueue;
}

void SMQServer::Connection::run()
{
    while (!terminated()) {
        try {
            shared_ptr<SMessageQueue> queue = subscribedQueue();
            SMessage message;
            if (queue && queue->pop(message, chrono::milliseconds(1000))) {
                SMQServer::sendMessage(socket(), *message);
            }
        }
        catch (const Exception& e) {
            CERR(e.what() << endl);
        }
    }
    socket().close();
    CERR("Connection terminated" << endl);
}

String SMQServer::Connection::clientId() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_clientId;
}

//-------------------------------------------------------------------------------

SMQServer::SMQServer(const String& username, const String& password, LogEngine& logEngine)
: TCPServer("SMQServer", 16, &logEngine),
  m_username(username), m_password(password),
  m_socketEvents(SMQServer::socketEventCallback, chrono::seconds(1))
{
}

ServerConnection* SMQServer::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return new Connection(*this, connectionSocket, peer);
}

void SMQServer::removeConnection(ServerConnection* connection)
{
    String clientId = dynamic_cast<Connection*>(connection)->clientId();
    lock_guard<mutex> lock(m_mutex);
    m_clientIds.erase(clientId);
}

void SMQServer::socketEventCallback(void *userData, SocketEventType eventType)
{
    Connection* connection = (Connection*) userData;

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
                        connection->terminate();
                    break;
                case Message::SUBSCRIBE:
                    connection->subscribeTo(msg->destination());
                    break;
                case Message::MESSAGE:
                    smqServer->distributeMessage(msg);
                    break;
                default:
                    break;
            }
        }
    }
    catch (const Exception& e) {
        connection->terminate();
        smqServer->log(LP_ERROR, e.message());
    }
}

void SMQServer::distributeMessage(SMessage message)
{
    shared_ptr<SMessageQueue> queue = getClientQueue(message->destination());
    queue->push(message);
}

shared_ptr<SMessageQueue> SMQServer::getClientQueue(const String& destination)
{
    lock_guard<mutex> lock(m_mutex);
    auto queue = m_queues[destination];
    if (!queue) {
        queue = make_shared<SMessageQueue>();
        m_queues[destination] = queue;
    }
    return queue;
}

bool SMQServer::authenticate(const String& clientId, const String& username, const String& password)
{
    lock_guard<mutex> lock(m_mutex);
    if (m_clientIds.find(clientId) != m_clientIds.end()) {
        log(LP_ERROR, "Duplicate client id: " + clientId);
        return false;
    }
    if (username != m_username || password != m_password) {
        log(LP_ERROR, "Invalid username or password, client id: " + clientId);
        return false;
    }
    return true;
}

void SMQServer::sendMessage(TCPSocket& socket, const Message& message)
{
    if (!socket.active())
        throw Exception("Not connected");

    Buffer output;

    // Append message type
    output.append((uint8_t)message.type());

    if (message.type() == Message::MESSAGE || message.type() == Message::SUBSCRIBE) {
        if (message.destination().empty())
            throw Exception("Empty destination");
        // Append destination
        output.append((uint8_t) message.destination().size());
        output.append(message.destination());
    }

    output.append((uint32_t)message.bytes());
    output.append(message.c_str(), message.bytes());

    const char* magic = "MSG:";
    socket.write(magic, strlen(magic));
    socket.write(output);
}

#if USE_GTEST

size_t messageCount {2};

TEST(SPTK_SMQServer, minimal)
{
    Buffer          buffer;
    FileLogEngine   logEngine("SMQServer.log");

    SMQServer smqServer("user", "secret", logEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient smqClient;
    ASSERT_NO_THROW(smqClient.connect(Host("localhost:4000"), "test-client1", "user", "secret"));

    smqClient.subscribe("test-queue");

    DateTime started("now");
    for (size_t m = 0; m < messageCount; m++)
        smqClient.sendMessage(Message(Message::MESSAGE, Buffer("This is SMQ test"), "test-queue"));

    size_t maxWait = 1000;
    while (smqClient.hasMessages() < messageCount) {
        this_thread::sleep_for(chrono::milliseconds(1));
        maxWait--;
        if (maxWait == 0)
            break;
    }

    DateTime ended("now");
    size_t durationMS = chrono::duration_cast<chrono::milliseconds>(ended - started).count();
    COUT("Done for " << durationMS << " ms, " << double(messageCount) / durationMS * 1000 << " msg/sec" << endl);

    smqClient.disconnect();
    smqServer.stop();
}

#endif
