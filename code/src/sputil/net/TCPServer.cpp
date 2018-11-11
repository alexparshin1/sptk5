/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPServer.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

using namespace std;
using namespace sptk;

TCPServer::TCPServer(Logger* logger)
: Thread("TCPServer"), m_listenerThread(nullptr), m_logger(logger)
{
    run();
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
    {
        UniqueLock(m_connectionThreadsLock);
        for (auto connectionThread: m_connectionThreads)
            connectionThread->terminate();
    }

    while (true) {
        this_thread::sleep_for(chrono::milliseconds(100));
        UniqueLock(m_connectionThreadsLock);
        if (m_connectionThreads.empty())
            break;
    }

    while (!m_completedConnectionThreads.empty()) {
        ServerConnection* connection;
        if (m_completedConnectionThreads.pop(connection, chrono::milliseconds(100)))
            delete connection;
    }

    if (m_listenerThread != nullptr) {
        m_listenerThread->terminate();
        m_listenerThread->join();
        delete m_listenerThread;
        m_listenerThread = nullptr;
    }
    terminate();
    join();
}

void TCPServer::registerConnection(ServerConnection* connection)
{
    UniqueLock(m_connectionThreadsLock);
    m_connectionThreads.insert(connection);
    connection->m_server = this;
}

void TCPServer::unregisterConnection(ServerConnection* connection)
{
    UniqueLock(m_connectionThreadsLock);
    m_connectionThreads.erase(connection);
    m_completedConnectionThreads.push(connection);
}

void TCPServer::threadFunction()
{
    chrono::seconds timeout(1);
    while (!terminated()) {
        ServerConnection* connection;
        if (m_completedConnectionThreads.pop(connection, timeout))
            delete connection;
    }
}

void TCPServer::terminate()
{
    m_completedConnectionThreads.wakeup();
    Thread::terminate();
}

#if USE_GTEST
#include <gtest/gtest.h>
#include <sptk5/net/TCPServerConnection.h>

/**
 * Not encrypted connection to control service
 */
class EchoConnection : public TCPServerConnection
{
public:
    EchoConnection(SOCKET connectionSocket, sockaddr_in* addr)
    : TCPServerConnection(connectionSocket)
    {
    }

    /**
     * Terminate connection thread
     */
    void terminate() override
    {
        m_socket->close();
        TCPServerConnection::terminate();
    }

    /**
     * Connection thread function
     */
    void threadFunction() override
    {
        Buffer data;
        while (!terminated()) {
            try {
                if (m_socket->readyToRead(chrono::seconds(30))) {
                    if (m_socket->readLine(data) == 0)
                        return;
                    string str(data.c_str());
                    str += "\n";
                    m_socket->write(str);
                } else
                    break;
            }
            catch (exception& e) {
                cerr << e.what() << endl;
            }
        }
        m_socket->close();
    }
};

class EchoServer : public sptk::TCPServer
{
protected:

    sptk::ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override
    {
        return new EchoConnection(connectionSocket, peer);
    }

public:

    EchoServer() {}

};

TEST(SPTK_TCPServer, minimal)
{
    Buffer buffer;

    EchoServer echoServer;
    ASSERT_NO_THROW(echoServer.listen(3000));

    TCPSocket socket;
    ASSERT_NO_THROW(socket.open(Host("localhost:3000")));

    Strings words("Hello, World!\n"
                  "This is a test of TCPServer class.\n"
                  "Using simple echo server to verify data flow.\n"
                  "The session is terminated when this row is received", "\n");

    for (auto& word: words) {
        socket.write(word + "\n");
        buffer.bytes(0);
        if (socket.readyToRead(chrono::seconds(3)))
            socket.readLine(buffer);
        EXPECT_STREQ(word.c_str(), buffer.c_str());
    }

    socket.close();
}

#endif
