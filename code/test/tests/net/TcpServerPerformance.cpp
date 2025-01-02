/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/net/SocketEvents.h"
#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

#include <netinet/tcp.h>

using namespace std;
using namespace sptk;

constexpr int testTcpEchoServerPort = 12345;

namespace {

Semaphore completed;
size_t    totalTransferredCount = 0;
size_t    totalTransferred = 0;

void eventHandler(const uint8_t* data, SocketEventType type)
{
    static int count = 0;

    auto& socket = *(Socket*) data;
    if (type.m_data)
    {
        size_t  size = 0;
        uint8_t buffer[1024];
        socket.read((uint8_t*) &size, sizeof(size));
        socket.read(buffer, size);
        totalTransferredCount++;
        totalTransferred += size + sizeof(size);
        buffer[size] = 0;
        socket.write((uint8_t*) &size, sizeof(size));
        socket.write(buffer, size);
        ++count;
        if (count > 1000)
        {
            completed.post();
        }
    }

    if (type.m_hangup)
    {
        socket.close();
    }
}

} // namespace

TEST(SPTK_TCPServer, SocketEventPerformance)
{
    SocketEvents socketEvents("Test Pool", eventHandler, 1s, SocketPool::TriggerMode::LevelTriggered);
    TCPSocket    clientSocket;

    TCPServer tcpServer("Performance Test Server");
    tcpServer.addListener(ServerConnection::Type::TCP, testTcpEchoServerPort);
    tcpServer.onConnection([&socketEvents, &clientSocket](ServerConnection& socket)
                           {
                               clientSocket.attach(socket.socket().detach(), false);
                               clientSocket.setOption(IPPROTO_TCP, TCP_NODELAY, 1);
                               clientSocket.blockingMode(false);
                               socketEvents.add(clientSocket, (uint8_t*) &clientSocket);
                           });

    TCPSocket socket;
    socket.open({"127.0.0.1", testTcpEchoServerPort});
    socket.setOption(IPPROTO_TCP, TCP_NODELAY, 1);
    socket.blockingMode(false);

    StopWatch stopWatch;
    stopWatch.start();

    size_t size = 21;
    socket.write((uint8_t*) &size, sizeof(size));
    socket.write((uint8_t*) "12345678901234567890", size);
    socketEvents.add(socket, (uint8_t*) &socket);

    completed.wait();

    stopWatch.stop();

    auto clientReceivedCount = totalTransferredCount / 2;
    auto clientReceivedBytes = totalTransferred / 2;
    COUT("Client received: " << clientReceivedBytes << " bytes for " << stopWatch.milliseconds() << " ms, "
                             << clientReceivedBytes / stopWatch.milliseconds() << "KB/s. (" << clientReceivedCount / stopWatch.milliseconds() << "K/s)");

    socketEvents.remove(socket);
    socketEvents.remove(clientSocket);
}
