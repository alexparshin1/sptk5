/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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
#include <poll.h>
#endif

using namespace std;
using namespace sptk;

namespace {

/**
 * @brief How often the waiter looks at connections that have not spoken yet.
 *
 * Short, because it also caps how long a connection deferred while the waiter is already polling
 * has to wait to be looked at. It costs 200 wake-ups a second while any connection is pending,
 * and nothing at all when none is: the waiter sleeps on a condition variable then.
 */
constexpr int pendingPollMs = 5;

/**
 * @brief How long a connection may stay open without saying anything.
 *
 * Generous: a browser opens connections before it knows whether it will use them, and closing
 * one it still means to use turns into a failed request.
 */
constexpr auto silentConnectionTimeout = std::chrono::seconds(60);

/**
 * @brief Whether the client has already sent something.
 *
 * Asked without waiting: the answer decides whether building the connection here would block.
 */
bool hasInput(const SocketType handle)
{
    pollfd descriptor {handle, POLLIN, 0};
#ifdef _WIN32
    const auto ready = WSAPoll(&descriptor, 1, 0);
#else
    const auto ready = ::poll(&descriptor, 1, 0);
#endif
    return ready > 0 && (descriptor.revents & POLLIN) != 0;
}

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

FastTcpServerListener::FastTcpServerListener(FastTCPServer& server, const Host& listenerHost, const ServerConnection::Type connectionType,
                                             const int backlog)
    : Thread("FastTcpServer::Listener")
    , m_server(server)
    , m_connectionType(connectionType)
    , m_backlog(backlog)
{
    m_listenerSocket.host(listenerHost);
}

void FastTcpServerListener::listen()
{
    if (!running())
    {
        m_listenerSocket.listen(0, true, m_backlog);
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

        // Drain the accepting backlog with consecutive accept() calls, without
        // re-polling between connections. This minimizes the number of syscalls per
        // accepted connection when connections arrive in bursts.
        const auto listenerFd = m_listenerSocket.fd();
        auto       acceptedAny = false;
        while (!terminated())
        {
            // sockaddr_storage, not sockaddr_in: an IPv6 (or dual-stack) peer's sockaddr_in6
            // is 28 bytes. A sockaddr_in-sized (16-byte) buffer would let the kernel truncate
            // the address, and downstream code that reinterprets it as sockaddr_in6 (e.g.
            // ServerConnection::parseAddress) would read past the end of the buffer, producing
            // garbled peer addresses in logs.
            sockaddr_storage connectionStorage = {};
            socklen_t        addressLength = sizeof(connectionStorage);
            const auto       connectionFD = ::accept(listenerFd, bit_cast<sockaddr*>(&connectionStorage), &addressLength);
            if (connectionFD == INVALID_SOCKET)
            {
                // Backlog drained (EWOULDBLOCK/EAGAIN) or transient error: stop draining.
                break;
            }
            m_server.acceptIncoming(m_connectionType, connectionFD, *bit_cast<const sockaddr_in*>(&connectionStorage));
            acceptedAny = true;
        }
        return acceptedAny;
    }
    catch (const Exception& e)
    {
        // stop() shuts the listener down to unblock this poll, which surfaces here as
        // "Connection closed". That is the expected way the thread wakes up on shutdown,
        // not a failure worth logging.
        if (!terminated())
        {
            m_server.log(LogPriority::Error, e.what());
        }
    }
    return false;
}

FastTCPServer::FastTCPServer(const std::string& serverName, std::shared_ptr<LogEngine> logEngine, SocketPoolTriggerMode triggerMode, const size_t maxEvents,
                             const int backlog)
    : m_logEngine(std::move(logEngine))
    , m_socketEvents(
          serverName,
          [this](const std::weak_ptr<ServerConnection>& weakConnection, SocketEventType type)
          {
              // The reactor delivers a weak_ptr; lock it here and keep the downstream
              // socketEventCallback(shared_ptr) interface unchanged.
              if (const auto connection = weakConnection.lock())
              {
                  socketEventCallback(connection, type);
              }
          },
          std::chrono::milliseconds(100), triggerMode, maxEvents)
    , m_backlog(backlog)
{
    if (m_logEngine)
    {
        m_logger = std::make_shared<Logger>(*m_logEngine);
    }

    m_pendingWaiter = JoiningThread([this] { waitForPendingConnections(); });
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

void FastTCPServer::waitForPendingConnections()
{
    while (true)
    {
        vector<PendingConnection> waiting;
        {
            unique_lock lock(m_pendingMutex);
            m_pendingArrived.wait_for(lock, chrono::milliseconds(pendingPollMs),
                                      [this] { return m_pendingStopping || !m_pending.empty(); });
            if (m_pendingStopping)
            {
                for (const auto& pending: m_pending)
                {
                    closeSocketHandle(pending.socket);
                }
                m_pending.clear();
                return;
            }
            waiting.swap(m_pending);
        }

        if (waiting.empty())
        {
            continue;
        }

        vector<pollfd> descriptors;
        descriptors.reserve(waiting.size());
        for (const auto& pending: waiting)
        {
            descriptors.push_back(pollfd {pending.socket, POLLIN, 0});
        }

#ifdef _WIN32
        (void) WSAPoll(descriptors.data(), static_cast<ULONG>(descriptors.size()), pendingPollMs);
#else
        (void) ::poll(descriptors.data(), descriptors.size(), pendingPollMs);
#endif

        const auto                now = chrono::steady_clock::now();
        vector<PendingConnection> stillWaiting;
        for (size_t index = 0; index < waiting.size(); ++index)
        {
            const auto& pending = waiting[index];
            if (descriptors[index].revents != 0)
            {
                // Spoken, or gone: a connection the client closed polls readable and then reads
                // nothing, and building it fails and closes it, which is the right end for it.
                buildConnection(pending.type, pending.socket, pending.peer);
            }
            else if (pending.deadline <= now)
            {
                // Held open and never used, so that the list cannot grow without bound.
                closeSocketHandle(pending.socket);
            }
            else
            {
                stillWaiting.push_back(pending);
            }
        }

        if (!stillWaiting.empty())
        {
            const scoped_lock lock(m_pendingMutex);
            m_pending.insert(m_pending.end(), stillWaiting.begin(), stillWaiting.end());
        }
    }
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
        const scoped_lock lock(m_pendingMutex);
        m_pendingStopping = true;
    }
    m_pendingArrived.notify_all();
    m_pendingWaiter.join();

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
        auto listener = make_shared<FastTcpServerListener>(*this, listenerHost, connectionType, m_backlog);
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

    // Encrypted, and nothing sent yet: building it would run the TLS handshake on this thread,
    // which is the listener's accept loop, and a client that says nothing would hold it there.
    // Handed to the waiter instead. Unencrypted connections are built here as they always were -
    // there is no handshake to block on, and this is the path that has to stay quick.
    if (connectionType == ServerConnection::Type::SSL && !hasInput(connectionFD))
    {
        {
            const scoped_lock lock(m_pendingMutex);
            m_pending.push_back({connectionType, connectionFD, peer,
                                 chrono::steady_clock::now() + silentConnectionTimeout});
        }
        m_pendingArrived.notify_one();
        return;
    }

    buildConnection(connectionType, connectionFD, peer);
}

void FastTCPServer::buildConnection(const ServerConnection::Type connectionType, const SocketType connectionFD,
                                    const sockaddr_in& peer)
{
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
        // socket (attached it to its own object) or rejected it. Do not close the FD;
        // otherwise we would close a descriptor now owned elsewhere.
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
        // Track the connection before arming the reactor: once it is armed, an event may fire on
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
    // unwatch it (e.g., the worker thread now processing it) and is re-tracked if it is re-watched.
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
