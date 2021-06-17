/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/net/UDPSocket.h>
#include <sptk5/threads/Thread.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

UDPSocket::UDPSocket(SOCKET_ADDRESS_FAMILY _domain)
 : BaseSocket(_domain, SOCK_DGRAM)
{
    setSocketFD(socket(domain(), type(), protocol()));
}

size_t UDPSocket::read(char *buffer, size_t size, sockaddr_in* from)
{
    sockaddr_in6 addr;
    if (from == nullptr)
        from = (sockaddr_in*)&addr;

    socklen_t addrLength = sizeof(sockaddr_in);
    auto bytes = recvfrom(fd(), buffer, (int) size, 0, (sockaddr*) from, &addrLength);
    if (bytes == -1)
        THROW_SOCKET_ERROR("Can't read from socket");
    return (size_t) bytes;
}

size_t UDPSocket::read(Buffer& buffer, size_t size, sockaddr_in* from)
{
    buffer.checkSize(size);
    socklen_t addrLength = sizeof(sockaddr_in);
    auto bytes = recvfrom(fd(), buffer.data(), (int) size, 0, (sockaddr*) from, &addrLength);
    if (bytes == -1)
        THROW_SOCKET_ERROR("Can't read from socket");
    buffer.bytes(bytes);
    return (size_t) bytes;
}

size_t UDPSocket::read(String& buffer, size_t size, sockaddr_in* from)
{
    buffer.resize(size);
    socklen_t addrLength = sizeof(sockaddr_in);
    auto bytes = recvfrom(fd(), buffer.data(), (int) size, 0, (sockaddr*) from, &addrLength);
    if (bytes == -1)
        THROW_SOCKET_ERROR("Can't read from socket");
    buffer.resize((size_t) bytes);
    return (size_t) bytes;
}

#if USE_GTEST

class UDPEchoServer : public UDPSocket, public Thread
{
    UDPSocket   socket;
public:
    UDPEchoServer()
    : Thread("UDP server")
    {
        socket.bind(nullptr, 3000);
    }

    void getAddress(sockaddr_in& addr) const
    {
        return socket.host().getAddress(addr);
    }

    /**
     * Terminate connection thread
     */
    void terminate() override
    {
        socket.close();
    }

    /**
     * Session thread function
     */
    void threadFunction() override
    {
        Buffer data(2048);
        while (!terminated()) {
            try {
                if (socket.readyToRead(chrono::seconds(30))) {
                    sockaddr_in from {};
                    size_t sz = socket.read(data.data(), 2048, &from);
                    if (sz == 0)
                        return;
                    data.bytes(sz);
                    socket.write((const uint8_t*) data.c_str(), sz, &from);
                }
            }
            catch (const Exception& e) {
                CERR(e.what() << endl)
            }
        }
        socket.close();
    }
};

TEST(SPTK_UDPSocket, minimal)
{
    Buffer buffer(4096);

    UDPEchoServer echoServer;
    echoServer.run();

    sockaddr_in serverAddr {};
    Host serverHost("127.0.0.1:3000");
    serverHost.getAddress(serverAddr);

    Strings rows("Hello, World!\n"
                 "This is a test of TCPServer class.\n"
                 "Using simple echo server to verify data flow.\n"
                 "The session is terminated when this row is received", "\n");


    UDPSocket socket;

    int rowCount = 0;
    for (const auto& row: rows) {
        socket.write((const uint8_t*) row.c_str(), row.length(), &serverAddr);
        buffer.bytes(0);
        if (socket.readyToRead(chrono::seconds(3))) {
            auto bytes = socket.read(buffer.data(), 2048);
            if (bytes > 0)
                buffer.bytes(bytes);
        }
        EXPECT_STREQ(row.c_str(), buffer.c_str());
        ++rowCount;
    }
    EXPECT_EQ(4, rowCount);

    socket.close();
}

#endif
