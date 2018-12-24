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
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

SMQServer::SMQServer()
: TCPServer("SMQServer", 16), m_socketEvents(SMQServer::socketEventCallback, chrono::seconds(1))
{
}

ServerConnection* SMQServer::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return new Connection(*this, connectionSocket, peer);
}

void SMQServer::distributeMessage(const String& destination, Message&& message)
{

}

SMQServer::Connection::Connection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*)
: TCPServerConnection(server, connectionSocket)
{
}

void SMQServer::Connection::terminate()
{
    socket().close();
    TCPServerConnection::terminate();
}

void SMQServer::Connection::readRawMessage(String& destination, Buffer& message)
{
    char data[16];
    socket().read(data, 4);
    if (strncmp(data, "MSG:", 4) != 0)
        throw Exception("Invalid message magic byte");

    uint32_t dataSize;
    socket().read((char*)&dataSize, sizeof(dataSize));
    if (dataSize == 0)
        throw Exception("Invalid message destination size");

    socket().read(destination, dataSize);

    socket().read((char*)&dataSize, sizeof(dataSize));
    if (dataSize == 0)
        throw Exception("Invalid message data size");

    message.checkSize(dataSize);
    socket().read(message.data(), dataSize);
    message.bytes(dataSize);
}

void SMQServer::socketEventCallback(void *userData, SocketEventType eventType)
{

}

void SMQServer::Connection::run()
{
    while (!terminated()) {
        try {
            if (socket().readyToRead(chrono::seconds(1))) {
                Buffer data;
                String destination;
                readRawMessage(destination, data);
                SMQServer* smqServer = dynamic_cast<SMQServer*>(&server());
                if (smqServer != nullptr)
                    smqServer->distributeMessage(destination, data);
            }
        }
        catch (const Exception& e) {
            CERR(e.what() << endl);
        }
    }
    socket().close();
}

#if USE_GTEST

TEST(SPTK_SMQServer, minimal)
{
    Buffer buffer;

    SMQServer smqServer;
    ASSERT_NO_THROW(smqServer.listen(4000));

    SMQClient smqClient;
    ASSERT_NO_THROW(smqClient.connect(Host("localhost:4000")));

    smqClient.sendMessage("test-queue", Message(Buffer("Hello, World!")));
    smqClient.sendMessage("test-queue", Message(Buffer("This is SMQ test")));

    this_thread::sleep_for(chrono::seconds(300));

    smqClient.disconnect();
    smqServer.stop();
}

#endif
