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

#pragma once

#include "sptk5/threads/ReadWriteLock.h"
#include "sptk5/threads/ReadWriteMutex.h"


#include <sptk5/Logger.h>
#include <sptk5/net/Host.h>
#include <sptk5/net/SSLKeys.h>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/threads/Flag.h>
#include <sptk5/threads/Thread.h>

#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace sptk {

class FastTCPServer;

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Internal FastTcpServer listener thread.
 *
 * Accepts incoming connections on a single Host:port and hands each
 * accepted socket to the server for connection creation and monitoring.
 */
class SP_EXPORT FastTcpServerListener
    : public Thread
{
public:
    /**
     * @brief Constructor.
     * @param server            Fast TCP server that owns this listener.
     * @param listenerHost      Listener host and port number.
     * @param connectionType    Connection type (TCP or SSL).
     */
    FastTcpServerListener(FastTCPServer& server, const Host& listenerHost, ServerConnection::Type connectionType);

    ~FastTcpServerListener() override = default;

    /**
     * @brief Start socket listening.
     */
    void listen();

    /**
     * @brief Returns the listener host.
     */
    Host host() const;

    /**
     * @brief Stop the running listener and join its thread.
     */
    void stop();

    /**
     * @brief Wait until the listener thread has started.
     * @param timeout           Maximum time to wait.
     */
    bool waitUntilStarted(std::chrono::milliseconds timeout)
    {
        return m_hasStarted.wait_for(true, timeout);
    }

protected:
    /**
     * @brief Listener thread function.
     */
    void threadFunction() override;

private:
    FastTCPServer&         m_server;             ///< Owning server.
    TCPSocket              m_listenerSocket;     ///< Listener socket.
    ServerConnection::Type m_connectionType;     ///< Connection type.
    Flag                   m_hasStarted {false}; ///< True once the listener thread has started.

    bool acceptConnection(const std::chrono::milliseconds& timeout);
};

/**
 * @brief High-performance event-driven TCP/SSL server.
 *
 * Unlike TCPServer, which runs a dedicated thread per connection, FastTcpServer
 * monitors all accepted connections with a single SocketEvents reactor
 * (epoll/kqueue/wepoll). When data arrives on a connection, the reactor invokes
 * socketEventCallback() so the application can process the incoming data without
 * the overhead of thread-per-connection scheduling.
 *
 * Usage:
 *   - Derive a server class and implement socketEventCallback().
 *   - Optionally override createConnection() and allowConnection().
 *   - Call addListener() for one or more Host:port combinations.
 *
 * @remarks Derived classes MUST call stop() from their destructor before any of
 *          their own members are destroyed, to guarantee the reactor thread is
 *          no longer delivering events into socketEventCallback().
 */
class SP_EXPORT FastTCPServer
{
    friend class FastTcpServerListener;

public:
    /**
     * @brief Constructor.
     * @param serverName        Logical name of the server (also the reactor thread name).
     * @param logEngine         Optional log engine.
     * @param triggerMode       Socket pool trigger mode.
     */
    explicit FastTCPServer(const std::string& serverName, std::shared_ptr<LogEngine> logEngine = nullptr, SocketPoolTriggerMode triggerMode = SocketPoolTriggerMode::LevelTriggered)
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
              std::chrono::milliseconds(100), triggerMode, 1024)
    {
        if (m_logEngine)
        {
            m_logger = std::make_shared<Logger>(*m_logEngine);
        }
    }

    /**
     * @brief Destructor.
     */
    virtual ~FastTCPServer();

    FastTCPServer(const FastTCPServer&) = delete;
    FastTCPServer& operator=(const FastTCPServer&) = delete;

    /**
     * @brief Start the server (ensure all listeners are listening).
     */
    void start();

    /**
     * @brief Stop the server: stop all listeners and close all connections.
     */
    void stop();

    /**
     * @brief Get server state.
     */
    bool active() const
    {
        const std::scoped_lock lock(m_mutex);
        return !m_listeners.empty();
    }

    /**
     * @brief Start a TCP or SSL listener on the selected host and port.
     * @remarks A listener may use several listener threads on the same Host:port
     *          combination (requires SO_REUSEPORT, which is enabled by default).
     * @param connectionType    Listener connection type.
     * @param listenerHost      Listener host and port number.
     * @param listenerCount     Number of listener threads.
     */
    void addListener(ServerConnection::Type connectionType, const Host& listenerHost, uint16_t listenerCount = 1);

    /**
     * @brief Remove the listener on the selected host and port.
     * @param listenerHost      Listener host and port number.
     */
    [[maybe_unused]] void removeListener(const Host& listenerHost);

    /**
     * @brief Get the listener hosts of the server.
     */
    std::vector<Host> listenerHosts() const;

    /**
     * @brief Set SSL keys for SSL connections (encrypted mode only).
     * @param sslKeys           SSL keys info.
     */
    void setSSLKeys(std::shared_ptr<SSLKeys> sslKeys);

    /**
     * @brief Get SSL keys for SSL connections (encrypted mode only).
     */
    std::shared_ptr<SSLKeys> getSSLKeys() const;

    /**
     * @brief Create a ServerConnection object for an accepted client connection.
     *
     * The default implementation wraps the accepted socket in a FastServerConnection.
     * Applications may override to create a custom ServerConnection-derived object.
     * @param connectionType    Incoming connection type.
     * @param connectionSocket  Already accepted incoming connection's socket handle.
     * @param peer              Incoming connection address.
     */
    virtual SServerConnection createConnection(ServerConnection::Type connectionType, const SocketType connectionSocket, const sockaddr_in* peer)
    {
        const STCPSocket socket = createConnectionSocket(connectionType, connectionSocket);

        auto connection = std::make_shared<ServerConnection>(connectionType, peer);
        connection->setSocket(socket);

        return connection;
    }

    /**
     * @brief Allow or deny an incoming connection.
     * The peer parameter is the incoming connection address.
     * @return true if the connection is allowed.
     */
    virtual bool allowConnection(const sockaddr_in* /*peer*/)
    {
        return true;
    }

    /**
     * @brief Stop monitoring and close a single client connection.
     * @param connection        Connection to close.
     */
    void closeConnection(const std::shared_ptr<ServerConnection>& connection);

    /**
     * @brief Stop monitoring and close all client connections.
     */
    void closeAllConnections();

    /**
     * @brief Number of currently monitored connections.
     */
    size_t connectionCount() const;

    /**
     * @brief Log a server message.
     * @param priority          Log message priority.
     * @param message           Log message.
     */
    void log(LogPriority priority, const String& message) const
    {
        if (m_logger)
        {
            m_logger->log(priority, message);
        }
    }

    /**
     * @brief Set socket events callback.
     *
     * Called by the reactor thread for every event on a monitored connection. The
     * implementation fully owns the event, including the connection lifecycle: it
     * should process available data (eventType.m_data) and tear the connection down
     * on peer hangup or error (eventType.m_hangup / eventType.m_error) using
     * closeConnection(). Failing to release a connection on hangup/error will keep
     * the level-triggered reactor re-signaling it.
     *
     * Use watchConnection() / unwatchConnection() to pause and resume monitoring (for
     * example, while a worker thread processes the connection).
     * @param eventCallback     Socket event callback.
     */
    void onSocketEvent(const SocketEventCallback<ServerConnection>& eventCallback)
    {
        m_socketEventCallback = eventCallback;
    }

    /**
     * @brief Start monitoring a connection for input events and track it.
     * @param connection        Connection to monitor.
     * @param rearm             Rearm connection (OneShot mode only).
     */
    void watchConnection(const std::shared_ptr<ServerConnection>& connection, bool rearm = false);

    /**
     * @brief Stop monitoring a connection for input events without closing it.
     * @param connection        Connection to stop monitoring.
     */
    void unwatchConnection(const std::shared_ptr<ServerConnection>& connection);

    /**
     * @return Socket events trigger mode.
     */
    SocketPoolTriggerMode getTriggerMode() const
    {
        return m_socketEvents.getTriggerMode();
    }

protected:
    /**
     * @brief Socket events callback.
     *
     * Called by the reactor thread for every event on a monitored connection. The
     * implementation fully owns the event, including the connection lifecycle: it
     * should process available data (eventType.m_data) and tear the connection down
     * on peer hangup or error (eventType.m_hangup / eventType.m_error) using
     * closeConnection(). Failing to release a connection on hangup/error will keep
     * the level-triggered reactor re-signaling it.
     *
     * Use watchConnection() / unwatchConnection() to pause and resume monitoring (for
     * example, while a worker thread processes the connection).
     * @param connection        Connection that received the event.
     * @param eventType         Event type.
     */
    virtual void socketEventCallback(const std::shared_ptr<ServerConnection>& connection, SocketEventType eventType)
    {
        if (m_socketEventCallback)
        {
            m_socketEventCallback(connection, eventType);
        }
    }

    /**
     * @brief Tune a freshly accepted connection socket.
     *
     * The default implementation enables TCP_NODELAY and switches the socket to
     * non-blocking mode (suitable for the event reactor). Override to customize,
     * for example, to keep the socket in blocking mode.
     * @param socket            Accepted connection socket.
     */
    virtual void tuneSocket(const STCPSocket& socket);

    /**
     * @brief Create the connection socket object.
     * @param connectionType Connection type.
     * @param connectionSocket Raw connection socket.
     * @return
     */
    STCPSocket createConnectionSocket(ServerConnection::Type connectionType, SocketType connectionSocket) const;

private:
    using SListener = std::shared_ptr<FastTcpServerListener>;
    using Listeners = std::vector<SListener>;

    mutable std::mutex                     m_mutex;               ///< Mutex that protects listeners and keys.
    mutable ReadWriteMutex                 m_connectionsMutex;    ///< Mutex that protects m_connections.
    /// Strong references to all currently monitored connections, keyed by their raw pointer (the
    /// same cookie the reactor stores). The reactor holds only a weak reference, so without this
    /// registry an accepted connection would be destroyed the moment acceptIncoming() returns and
    /// no event would ever be delivered. A connection lives here from watchConnection() until
    /// unwatchConnection()/closeConnection().
    std::unordered_map<ServerConnection*, std::shared_ptr<ServerConnection>> m_connections;
    std::shared_ptr<LogEngine>             m_logEngine;           ///< Optional log engine.
    std::shared_ptr<Logger>                m_logger;              ///< Optional logger.
    SocketEvents<ServerConnection>         m_socketEvents;        ///< Socket events reactor.
    std::shared_ptr<SSLKeys>               m_keys;                ///< Server SSL keys.
    std::map<Host, Listeners, HostCompare> m_listeners;           ///< Server listeners.
    SocketEventCallback<ServerConnection>  m_socketEventCallback; ///< Optional socket event callback.

    /**
     * @brief Accept an incoming connection (called by the listener thread).
     */
    void acceptIncoming(ServerConnection::Type connectionType, SocketType connectionFD, const sockaddr_in& peer);
};

/**
 * @}
 */

} // namespace sptk
