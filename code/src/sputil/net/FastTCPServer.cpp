/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/SSLSocket.h>

#include <filesystem>
#include <ranges>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

namespace {

/**
 * @brief Hard-close a raw socket handle for a connection that is rejected or fails to initialize.
 */
void closeSocketHandle(SocketType connectionFD)
{
    if (connectionFD == INVALID_SOCKET)
    {
        return;
    }
#ifndef _WIN32
    shutdown(connectionFD, SHUT_RDWR);
    ::close(connectionFD);
#else
    closesocket(connectionFD);
#endif
}

} // namespace

// ─────────────────────────────────────────── FastTcpServerListener ───────────────────────────────────────────

FastTcpServerListener::FastTcpServerListener(FastTCPServer& server, const Host& listenerHost, const ServerConnection::Type connectionType)
    : Thread("FastTcpServer::Listener")
    , m_server(server)
    , m_connectionType(connectionType)
{
    m_listenerSocket.host(listenerHost);
}

void FastTcpServerListener::listen()
{
    if (!running())
    {
        m_listenerSocket.listen(0, true);
        run();
    }
}

Host FastTcpServerListener::host() const
{
    return m_listenerSocket.host();
}

void FastTcpServerListener::stop()
{
    terminate();
    if (m_listenerSocket.active())
    {
        m_listenerSocket.close();
    }
    join();
}

bool FastTcpServerListener::acceptConnection(const chrono::milliseconds& timeout)
{
    try
    {
        // Wait for at least one pending connection (or terminate timeout).
        if (!m_listenerSocket.readyToRead(timeout))
        {
            return false;
        }

        // Drain the whole accept backlog with consecutive accept() calls, without
        // re-polling between connections. This minimizes the number of syscalls per
        // accepted connection when connections arrive in bursts.
        const auto listenerFd = m_listenerSocket.fd();
        bool       acceptedAny = false;
        while (!terminated())
        {
            sockaddr_in      connectionInfo = {};
            socklen_t        addressLength = sizeof(connectionInfo);
            const SocketType connectionFD = ::accept(listenerFd, bit_cast<sockaddr*>(&connectionInfo), &addressLength);
            if (connectionFD == INVALID_SOCKET)
            {
                // Backlog drained (EWOULDBLOCK/EAGAIN) or transient error: stop draining.
                break;
            }
            m_server.acceptIncoming(m_connectionType, connectionFD, connectionInfo);
            acceptedAny = true;
        }
        return acceptedAny;
    }
    catch (const Exception& e)
    {
        m_server.log(LogPriority::Error, e.what());
    }
    return false;
}

void FastTcpServerListener::threadFunction()
{
    try
    {
        // Indicate that the listener thread has started:
        m_hasStarted = true;

        // Short poll timeout so the listener reacts quickly to terminate() on shutdown.
        // Incoming connections still wake the poll immediately; the timeout only bounds
        // how often the terminated() flag is re-checked.
        constexpr auto acceptTimeout = chrono::milliseconds(100);

        m_listenerSocket.blockingMode(false);

        while (!terminated() && m_listenerSocket.active())
        {
            if (!acceptConnection(acceptTimeout) && !m_listenerSocket.active())
            {
                break;
            }
        }

        if (m_listenerSocket.active())
        {
            m_listenerSocket.close();
        }
    }
    catch (const Exception& e)
    {
        m_server.log(LogPriority::Error, e.what());
    }
}

// ───────────────────────────────────────────────── FastTcpServer ─────────────────────────────────────────────

FastTCPServer::~FastTCPServer()
{
    FastTCPServer::stop();
}

void FastTCPServer::start()
{
    const scoped_lock lock(m_mutex);
    for (const auto& listeners: m_listeners | views::values)
    {
        for (const auto& listener: listeners)
        {
            listener->listen();
        }
    }
}

void FastTCPServer::stop()
{
    {
        const scoped_lock lock(m_mutex);
        for (const auto& listeners: m_listeners | views::values)
        {
            for (const auto& listener: listeners)
            {
                listener->stop();
            }
        }
        m_listeners.clear();
    }

    closeAllConnections();
}

void FastTCPServer::addListener(const ServerConnection::Type connectionType, const Host& listenerHost, uint16_t listenerCount)
{
    const scoped_lock lock(m_mutex);

    if (connectionType == ServerConnection::Type::SSL && !m_keys)
    {
        throw Exception("Cannot add SSL listener: server SSL keys are not set");
    }

    auto& listeners = m_listeners[listenerHost];
    if (!listeners.empty())
    {
        throw Exception("Port is already used");
    }

    if (listenerCount == 0)
    {
        listenerCount = 1;
    }

    for (uint16_t i = 0; i < listenerCount; ++i)
    {
        auto listener = make_shared<FastTcpServerListener>(*this, listenerHost, connectionType);
        listeners.push_back(listener);
        listener->listen();
    }

    for (const auto& listener: listeners)
    {
        listener->waitUntilStarted(100ms);
    }
}

[[maybe_unused]] void FastTCPServer::removeListener(const Host& listenerHost)
{
    const scoped_lock lock(m_mutex);

    const auto itor = m_listeners.find(listenerHost);
    if (itor == m_listeners.end())
    {
        return;
    }

    for (const auto& listener: itor->second)
    {
        listener->stop();
    }

    m_listeners.erase(itor);
}

vector<Host> FastTCPServer::listenerHosts() const
{
    vector<Host> hosts;

    const scoped_lock lock(m_mutex);
    for (const auto& host: m_listeners | views::keys)
    {
        hosts.push_back(host);
    }

    return hosts;
}

void FastTCPServer::setSSLKeys(shared_ptr<SSLKeys> sslKeys)
{
    const scoped_lock lock(m_mutex);
    if (sslKeys && !filesystem::exists(sslKeys->certificateFileName()))
    {
        throw Exception("Can't find certificate file: " + sslKeys->certificateFileName().string());
    }
    m_keys = std::move(sslKeys);
}

shared_ptr<SSLKeys> FastTCPServer::getSSLKeys() const
{
    const scoped_lock lock(m_mutex);
    return m_keys;
}

STCPSocket FastTCPServer::createConnectionSocket(const ServerConnection::Type connectionType, const SocketType connectionSocket) const
{
    STCPSocket socket;

    if (connectionType == ServerConnection::Type::SSL)
    {
        const auto sslSocket = make_shared<SSLSocket>();
        const auto keys = getSSLKeys();
        if (!keys)
        {
            throw Exception("SSL connection can't be created as server has no SSL keys configured");
        }
        sslSocket->loadKeys(*keys);
        // Performs the server-side TLS handshake (blocking) on the accepted socket.
        sslSocket->attach(connectionSocket, true);
        socket = sslSocket;
    }
    else
    {
        socket = make_shared<TCPSocket>();
        socket->attach(connectionSocket, false);
    }
    return socket;
}

void FastTCPServer::acceptIncoming(const ServerConnection::Type connectionType, const SocketType connectionFD, const sockaddr_in& peer)
{
    if (!allowConnection(&peer))
    {
        closeSocketHandle(connectionFD);
        return;
    }

    shared_ptr<ServerConnection> connection;
    try
    {
        connection = createConnection(connectionType, connectionFD, &peer);
    }
    catch (const Exception& e)
    {
        log(LogPriority::Error, e.what());
        closeSocketHandle(connectionFD);
        return;
    }

    if (!connection)
    {
        // createConnection() returned no connection: it has taken ownership of the
        // socket (attached it to its own object) or rejected it. Do not close the FD,
        // otherwise we would close a descriptor that is now owned elsewhere.
        return;
    }

    const auto socket = connection->getSocket();
    if (!socket)
    {
        return;
    }

    tuneSocket(socket);
    watchConnection(connection);
}

void FastTCPServer::tuneSocket(const STCPSocket& socket)
{
    // Tune the connection for low-latency, event-driven I/O.
    try
    {
        socket->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
    }
    catch (const Exception&)
    {
        // TCP_NODELAY is best-effort; ignore failures.
    }
    socket->blockingMode(false);
}

void FastTCPServer::watchConnection(const shared_ptr<ServerConnection>& connection, const bool rearm)
{
    const auto socket = connection->getSocket();
    if (!socket)
    {
        return;
    }

    {
        // Track the connection before arming the reactor: once it is armed an event may fire on
        // another thread and look the connection up by raw pointer, so it must already be here.
        const WriteLock lock(m_connectionsMutex);
        m_connections[connection.get()] = connection;
    }

    try
    {
        m_socketEvents.add(socket, connection, rearm);
    }
    catch (const Exception& e)
    {
        log(LogPriority::Error, e.what());
        closeConnection(connection);
    }
}

void FastTCPServer::unwatchConnection(const shared_ptr<ServerConnection>& connection)
{
    const auto socket = connection->getSocket();
    if (!socket)
    {
        return;
    }

    try
    {
        m_socketEvents.remove(socket);
    }
    catch (const Exception&)
    {
        // Already removed or never added.
    }

    // Drop the server's strong reference. The connection stays alive through whoever asked to
    // unwatch it (e.g. the worker thread now processing it) and is re-tracked if it is re-watched.
    const WriteLock lock(m_connectionsMutex);
    m_connections.erase(connection.get());
}

void FastTCPServer::closeConnection(const shared_ptr<ServerConnection>& connection)
{
    if (!connection)
    {
        return;
    }

    const auto socket = connection->getSocket();
    if (!socket)
    {
        return;
    }

    try
    {
        m_socketEvents.remove(socket);
    }
    catch (const Exception&)
    {
        // Already removed or never added.
    }

    socket->close();

    const WriteLock lock(m_connectionsMutex);
    m_connections.erase(connection.get());
}

void FastTCPServer::closeAllConnections()
{
    // Move the registry out under the lock, then tear the connections down without holding it:
    // closeConnection()/the reactor acquire the pool lock and then this one, so holding this lock
    // across a pool call would invert that order and risk a deadlock.
    std::unordered_map<ServerConnection*, std::shared_ptr<ServerConnection>> connections;
    {
        const WriteLock lock(m_connectionsMutex);
        connections.swap(m_connections);
    }

    for (const auto& connection: connections | views::values)
    {
        const auto socket = connection->getSocket();
        if (!socket)
        {
            continue;
        }
        try
        {
            m_socketEvents.remove(socket);
        }
        catch (const Exception&)
        {
            // Already removed or never added.
        }
        socket->close();
    }
}

size_t FastTCPServer::connectionCount() const
{
    const ReadLock lock(m_connectionsMutex);
    return m_connections.size();
}
