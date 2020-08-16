/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

const map<String,LogDetails::MessageDetail> LogDetails::detailNames {
    { "source_ip", SOURCE_IP },
    { "request_name", REQUEST_NAME },
    { "request_duration", REQUEST_DURATION },
    { "request_data", REQUEST_DATA },
    { "response_data", RESPONSE_DATA }
};

LogDetails::LogDetails(const Strings& details)
{
    for (auto& detailName: details) {
        auto itor = detailNames.find(detailName);
        if (itor == detailNames.end())
            continue;
        m_details.set(itor->second);
    }
}

String LogDetails::toString(const String& delimiter) const
{
    Strings names;
    for (auto& itor: detailNames) {
        if (m_details.test(itor.second))
            names.push_back(itor.first);
    }
    return names.join(delimiter.c_str());
}


TCPServer::TCPServer(const String& listenerName, size_t threadLimit, LogEngine* logEngine, const LogDetails& logDetails)
: ThreadPool((uint32_t)threadLimit, std::chrono::seconds(60), listenerName),
  m_logDetails(logDetails)
{
    if (logEngine != nullptr)
        m_logger = make_shared<Logger>(*logEngine);

    char hostname[128] = { "localhost" };
    int rc = gethostname(hostname, sizeof(hostname));
    if (rc == 0)
        m_host = Host(hostname);
}

TCPServer::~TCPServer()
{
    TCPServer::stop();
}

const Host& TCPServer::host() const
{
    SharedLock(m_mutex);
    return m_host;
}

void TCPServer::host(const Host& host)
{
    SharedLock(m_mutex);
    if (!m_listenerThread)
        m_host = host;
}

void TCPServer::listen(uint16_t port)
{
    UniqueLock(m_mutex);
    m_host.port(port);
    m_listenerThread = make_shared<TCPServerListener>(this, port);
    m_listenerThread->listen();
}

bool TCPServer::allowConnection(sockaddr_in*)
{
    return true;
}

void TCPServer::stop()
{
    UniqueLock(m_mutex);
    ThreadPool::stop();
    m_listenerThread.reset();
}

void TCPServer::setSSLKeys(shared_ptr<SSLKeys> sslKeys)
{
    UniqueLock(m_mutex);
    m_sslKeys = sslKeys;
}

const SSLKeys& TCPServer::getSSLKeys() const
{
    SharedLock(m_mutex);
    return *m_sslKeys;
}

void TCPServer::threadEvent(Thread* thread, ThreadEvent::Type eventType, Runable* runable)
{
    if (eventType == RUNABLE_FINISHED) {
        delete runable;
        runable = nullptr;
    }
    ThreadPool::threadEvent(thread, eventType, runable);
}

#if USE_GTEST

/**
 * Not encrypted connection to control service
 */
class EchoConnection : public TCPServerConnection
{
public:
    EchoConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress)
    : TCPServerConnection(server, connectionSocket, connectionAddress)
    {
    }

    ~EchoConnection() override
    {
        COUT("Connection destroyed" << endl)
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
                CERR(e.what() << endl)
            }
        }
        socket().close();
    }
};

class EchoServer : public sptk::TCPServer
{
public:

    EchoServer() : TCPServer("EchoServer")
    {}

protected:

    sptk::ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override
    {
        return new EchoConnection(*this, connectionSocket, peer);
    }
};

TEST(SPTK_TCPServer, minimal)
{
    Buffer buffer;

    EchoServer echoServer;
    ASSERT_NO_THROW(echoServer.listen(3001));

    TCPSocket socket;
    ASSERT_NO_THROW(socket.open(Host("localhost", 3001)));

    Strings rows("Hello, World!\n"
                  "This is a test of TCPServer class.\n"
                  "Using simple echo server to verify data flow.\n"
                  "The session is terminated when this row is received", "\n");

    int rowCount = 0;
    for (const auto& row: rows) {
        socket.write(row + "\n");
        buffer.bytes(0);
        if (socket.readyToRead(chrono::seconds(3)))
            socket.readLine(buffer);
        EXPECT_STREQ(row.c_str(), buffer.c_str());
        ++rowCount;
    }
    EXPECT_EQ(4, rowCount);

    socket.close();
}

#endif
