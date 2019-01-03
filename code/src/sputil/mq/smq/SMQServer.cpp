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

#include "SMQServer.h"
#include "SMQClient.h"
#include "SMQMessage.h"
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

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

void SMQServer::Connection::readConnect()
{
    uint32_t    messageSize;
    String      username;
    String      password;
    SMQMessage::read(socket(), messageSize);
    SMQMessage::read(socket(), username);
    SMQMessage::read(socket(), password);

    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    if (!smqServer->authenticate(username, password))
        terminate();
}

void SMQServer::Connection::readMessage(String& destination, Buffer& message)
{
    SMQMessage::read(socket(), destination);
    SMQMessage::read(socket(), message);
}

void SMQServer::Connection::readRawMessage(String& destination, Buffer& message, uint8_t& messageType)
{
    char data[16];

    // Read message signature
    socket().read(data, 4);
    if (strncmp(data, "MSG:", 4) != 0)
        throw Exception("Invalid message magic byte");

    // Read message type
    socket().read((char*)&messageType, sizeof(messageType));
    if (messageType > Message::MESSAGE)
        throw Exception("Invalid message type");

    switch (messageType) {
        case Message::CONNECT:
            readConnect();
            break;
        case Message::MESSAGE:
        case Message::SUBSCRIBE:
        case Message::UNSUBSCRIBE:
            readMessage(destination, message);
            break;
        default:
            destination = "";
            message.bytes(0);
            break;
    }
}

void SMQServer::socketEventCallback(void *userData, SocketEventType eventType)
{
    Connection* connection = (Connection*) userData;
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&connection->server());

    try {
        while (connection->socket().socketBytes() > 0) {
            uint8_t messageType;
            Buffer data;
            String destination;

            connection->readRawMessage(destination, data, messageType);
            auto msg = make_shared<Message>((Message::Type) messageType, move(data));

            switch (messageType) {
                case Message::SUBSCRIBE:
                    connection->subscribeTo(destination);
                    break;
                case Message::MESSAGE:
                    msg->destination(destination);
                    smqServer->distributeMessage(destination, msg);
                    break;
            }
        }
    }
    catch (const Exception& e) {
        connection->terminate();
        smqServer->log(LP_ERROR, e.message());
    }
}

void SMQServer::distributeMessage(const String& destination, SMQServer::SMessage message)
{
    shared_ptr<SMessageQueue> queue = getClientQueue(destination);
    queue->push(message);
}

void SMQServer::Connection::subscribeTo(const String& destination)
{
    lock_guard<mutex> lock(m_mutex);
    SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
    m_subscribedQueue = smqServer->getClientQueue(destination);
}

shared_ptr<SMQServer::SMessageQueue> SMQServer::Connection::subscribedQueue()
{
    lock_guard<mutex> lock(m_mutex);
    return m_subscribedQueue;
}

shared_ptr<SMQServer::SMessageQueue> SMQServer::getClientQueue(const String& destination)
{
    lock_guard<mutex> lock(m_mutex);
    auto queue = m_queues[destination];
    if (!queue) {
        queue = make_shared<SMQServer::SMessageQueue>();
        m_queues[destination] = queue;
    }
    return queue;
}

bool SMQServer::authenticate(const String& username, const String& password)
{
    lock_guard<mutex> lock(m_mutex);
    return username == m_username && password == m_password;
}

void SMQServer::Connection::run()
{
    while (!terminated()) {
        try {
            shared_ptr<SMQServer::SMessageQueue> queue = subscribedQueue();
            SMessage message;
            if (queue && queue->pop(message, chrono::milliseconds(1000))) {
                cout << message->c_str() << endl;
                SMQServer::sendMessage(socket(), *message);
            }
        }
        catch (const Exception& e) {
            CERR(e.what() << endl);
        }
    }
    socket().close();
    cerr << "Connection terminated" << endl;
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

TEST(SPTK_SMQServer, minimal)
{
    Buffer          buffer;
    SysLogEngine    sysLogEngine;

    SMQServer smqServer("user", "secret", sysLogEngine);
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient smqClient;
    ASSERT_NO_THROW(smqClient.connect(Host("localhost:4000"), "user", "secret"));

    smqClient.subscribe("test-queue");
    smqClient.sendMessage(Message(Message::MESSAGE, Buffer("Hello, World!"), "test-queue"));
    smqClient.sendMessage(Message(Message::MESSAGE, Buffer("This is SMQ test"), "test-queue"));

    this_thread::sleep_for(chrono::seconds(300));

    smqClient.disconnect();
    smqServer.stop();
}

#endif
