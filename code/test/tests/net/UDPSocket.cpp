/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/net/UDPSocket.h>

using namespace std;
using namespace sptk;

static constexpr uint16_t testPort = 3000;
static constexpr uint16_t bufferSize = 2048;
static constexpr auto readTimeout = chrono::milliseconds(200);

class UDPEchoServer
    : public UDPSocket
    , public Thread
{
    UDPSocket socket;

public:
    UDPEchoServer()
        : Thread("UDP server")
    {
        socket.bind(nullptr, testPort);
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
        constexpr chrono::seconds timeout {5};
        DateTime stopTime = DateTime::Now() + timeout;
        Buffer data(bufferSize);
        while (!terminated() && DateTime::Now() < stopTime)
        {
            try
            {
                if (socket.readyToRead(readTimeout))
                {
                    sockaddr_in from {};
                    size_t sz = socket.read(data.data(), bufferSize, &from);
                    if (sz == 0)
                    {
                        return;
                    }
                    data.bytes(sz);
                    socket.write((const uint8_t*) data.c_str(), sz, &from);
                }
            }
            catch (const Exception& e)
            {
                COUT("Server: " << e.what() << endl)
                break;
            }
        }
        socket.close();
    }
};

TEST(SPTK_UDPSocket, minimal)
{
    Buffer buffer(bufferSize);

    UDPEchoServer echoServer;
    echoServer.run();

    sockaddr_in serverAddr {};
    Host serverHost("127.0.0.1", testPort);
    serverHost.getAddress(serverAddr);

    Strings rows("Hello, World!\n"
                 "This is a test of TCPServer class.\n"
                 "Using simple echo server to verify data flow.\n"
                 "The session is terminated when this row is received",
                 "\n");


    UDPSocket socket;

    int rowCount = 0;
    for (const auto& row: rows)
    {
        socket.write((const uint8_t*) row.c_str(), row.length(), &serverAddr);
        buffer.bytes(0);
        if (socket.readyToRead(readTimeout))
        {
            auto bytes = socket.read(buffer.data(), bufferSize);
            if (bytes > 0)
            {
                buffer.bytes(bytes);
            }
            COUT("received " << bytes << " bytes: " << buffer.c_str() << endl)
        }
        EXPECT_STREQ(row.c_str(), buffer.c_str());
        ++rowCount;
    }
    EXPECT_EQ(4, rowCount);

    echoServer.terminate();
    echoServer.join();

    socket.close();
}
