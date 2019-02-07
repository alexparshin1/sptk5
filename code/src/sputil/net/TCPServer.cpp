/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPServer.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cutils>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>
#if USE_GTEST
#include <sptk5/net/TCPServerConnection.h>
#endif

using namespace std;
using namespace sptk;

TCPServer::TCPServer(const String& listenerName, size_t threadLimit, LogEngine* logEngine)
: ThreadPool(threadLimit, std::chrono::seconds(60), listenerName), m_listenerThread(nullptr)
{
    if (logEngine != nullptr)
        m_logger = make_shared<Logger>(*logEngine);
}

TCPServer::~TCPServer()
{
    stop();
}

uint16_t TCPServer::port() const
{
    if (!m_listenerThread)
        return 0;
    return m_listenerThread->port();
}

void TCPServer::listen(uint16_t port)
{
    if (!running())
        run();

    UniqueLock(m_mutex);
    if (m_listenerThread != nullptr) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
    }

    m_listenerThread = new TCPServerListener(this, port);
    m_listenerThread->listen();
    m_listenerThread->run();
}

bool TCPServer::allowConnection(sockaddr_in*)
{
    return true;
}

void TCPServer::stop()
{
    UniqueLock(m_mutex);
    ThreadPool::stop();

    if (m_listenerThread != nullptr) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
        m_listenerThread = nullptr;
    }
}

#if USE_GTEST

/**
 * Not encrypted connection to control service
 */
class EchoConnection : public TCPServerConnection
{
public:
    EchoConnection(TCPServer& server, SOCKET connectionSocket, sockaddr_in*)
    : TCPServerConnection(server, connectionSocket)
    {
    }

    /**
     * Terminate connection thread
     */
    void terminate() override
    {
        socket().close();
        TCPServerConnection::terminate();
    }

    /**
     * Connection thread function
     */
    void run() override
    {
        Buffer data;
        while (!terminated()) {
            try {
                if (socket().readyToRead(chrono::seconds(30))) {
                    if (socket().readLine(data) == 0)
                        return;
                    string str(data.c_str());
                    str += "\n";
                    socket().write(str);
                } else
                    break;
            }
            catch (const Exception& e) {
                CERR(e.what() << endl);
            }
        }
        socket().close();
    }
};

class EchoServer : public sptk::TCPServer
{
protected:

    sptk::ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override
    {
        return new EchoConnection(*this, connectionSocket, peer);
    }

public:

    EchoServer() : TCPServer("EchoServer", 16) {}

};

TEST(SPTK_TCPServer, minimal)
{
    Buffer buffer;

    EchoServer echoServer;
    ASSERT_NO_THROW(echoServer.listen(3000));

    TCPSocket socket;
    ASSERT_NO_THROW(socket.open(Host("localhost:3000")));

    Strings rows("Hello, World!\n"
                  "This is a test of TCPServer class.\n"
                  "Using simple echo server to verify data flow.\n"
                  "The session is terminated when this row is received", "\n");

    int rowCount = 0;
    for (auto& row: rows) {
        socket.write(row + "\n");
        buffer.bytes(0);
        if (socket.readyToRead(chrono::seconds(3)))
            socket.readLine(buffer);
        EXPECT_STREQ(row.c_str(), buffer.c_str());
        rowCount++;
    }
    EXPECT_EQ(4, rowCount);

    socket.close();
}

#endif
