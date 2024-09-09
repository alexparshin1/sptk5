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

#pragma once

#include <bitset>
#include <set>
#include <sptk5/Logger.h>
#include <sptk5/net/SSLKeys.h>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/threads/ThreadPool.h>
#include <utility>

namespace sptk {

class TCPServerListener;

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * Log details
 *
 * Defines information about server activities that should be logged.
 */
class SP_EXPORT LogDetails
{
public:
    /**
     * Log details constants
     */
    enum class MessageDetail : uint8_t
    {
        SERIAL_ID,
        SOURCE_IP,
        REQUEST_NAME,
        REQUEST_DURATION,
        REQUEST_DATA,
        RESPONSE_DATA,
        THREAD_POOLING
    };

    using MessageDetails = std::set<MessageDetail>;

    /**
     * Default constructor
     */
    LogDetails() = default;

    /**
     * Constructor
     * @param details           Log details
     */
    explicit LogDetails(MessageDetails details)
        : m_details(std::move(details))
    {
    }

    /**
     * Constructor
     * @param details           Log details as lower case strings
     */
    explicit LogDetails(const Strings& details);

    /**
     * Constructor
     * @param details           Log details
     */
    LogDetails(std::initializer_list<MessageDetail> details)
    {
        for (auto detail: details)
        {
            m_details.insert(detail);
        }
    }

    [[nodiscard]] String toString(const String& delimiter = ",") const;

    /**
     * Query log details
     * @param detail            Log detail
     * @return true if log detail is set
     */
    [[nodiscard]] bool has(MessageDetail detail) const
    {
        return m_details.contains(detail);
    }

    [[nodiscard]] bool empty() const
    {
        return m_details.empty();
    }

private:
    MessageDetails                               m_details; ///< Log details set
    static const std::map<String, MessageDetail> detailNames;
};

/**
 * TCP server
 *
 * For every incoming connection, creates connection thread.
 */
class SP_EXPORT TCPServer
    : public ThreadPool
{
    friend class TCPServerListener;
    friend class ServerConnection;

public:
    /**
     * Constructor
     * @param listenerName      Logical name of the listener
     * @param threadLimit       Number of worker threads in thread pool
     * @param logEngine         Optional log engine
     * @param logDetails        Log messages details
     */
    explicit TCPServer(const String& listenerName, size_t threadLimit = 16, LogEngine* logEngine = nullptr,
                       const LogDetails& logDetails = LogDetails());

    /**
     * Destructor
     */
    ~TCPServer() override;

    /**
     * Get current host of the server
     * @return
     */
    const Host& host() const;

    /**
     * @brief Start TCP or SSL listener on the selected port
     * @param connectionType    Listener connection type
     * @param port              Listener port number
     * @param threadCount       Number of listener threads
     */
    void addListener(ServerConnection::Type connectionType, uint16_t port, uint16_t threadCount = 1);

    /**
     * @brief Remove listener on the selected port
     * @param port              Listener port number
     */
    [[maybe_unused]] void removeListener(uint16_t port);

    /**
     * @brief Stop and remove all listeners
     */
    void stop() override;

    /**
     * @brief Get server state
     */
    bool active() const
    {
        return !m_portListeners.empty();
    }

    /**
     * @brief Log server message
     * @param priority          Log message priority
     * @param message           Log message
     */
    void log(LogPriority priority, const String& message) const
    {
        if (m_logger)
        {
            m_logger->log(priority, message);
        }
    }

    /**
     * @brief Get server log details
     * @return current log details
     */
    [[maybe_unused]] const LogDetails& logDetails() const
    {
        return m_logDetails;
    }

    /**
     * @brief Set SSL keys for SSL connections (encrypted mode only)
     * @param sslKeys           SSL keys info
     */
    void setSSLKeys(std::shared_ptr<SSLKeys> sslKeys);

    /**
     * @brief Get SSL keys for SSL connections (encrypted mode only)
     * @return SSL keys info
     */
    const SSLKeys& getSSLKeys() const;

    /**
     * @brief Create connection thread derived from TCPServerConnection or SSLServerConnection
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param function          User-defined function that is called upon client connection to server
     */
    virtual void onConnection(const ServerConnection::Function& function);

protected:
    /**
     * @brief Modify server host.
     * If listener is already active, don't modify exiting server host.
     * @param host              Server host
     */
    void host(const Host& host);

    /**
     * @brief Screen incoming connection request
     * Method is called right after connection request is accepted,
     * and allows ignoring unwanted connections. By default simply returns true (allow).
     * @param connectionRequest Incoming connection information
     */
    virtual bool allowConnection(sockaddr_in* connectionRequest);

    /**
     * @brief Create connection thread derived from TCPServerConnection or SSLServerConnection
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param connectionType    Connection type
     * @param connectionSocket  Already accepted incoming connection socket
     * @param peer              Incoming connection information
     */
    virtual UServerConnection createConnection(ServerConnection::Type connectionType, SocketType connectionSocket, const sockaddr_in* peer);

    /**
     * Thread event callback function
     *
     * Receives events that occur in the threads
     * @param thread            Thread where event occured
     * @param eventType         Thread event type
     * @param runable           Related runable (if any)
     */
    void threadEvent(Thread* thread, ThreadEvent::Type eventType, SRunable runable) override;

private:
    using UListener = std::unique_ptr<TCPServerListener>;
    using Listeners = std::vector<UListener>;
    mutable std::mutex            m_mutex;              ///< Mutex protecting internal data
    std::map<uint16_t, Listeners> m_portListeners;      ///< Server port listeners
    std::shared_ptr<Logger>       m_logger;             ///< Optional logger
    std::shared_ptr<SSLKeys>      m_sslKeys;            ///< Optional SSL keys. Only used for SSL server.
    Host                          m_host;               ///< This host
    LogDetails                    m_logDetails;         ///< Log details
    ServerConnection::Function    m_connectionFunction; ///< User-defined function that is called upon client connection to server
};

/**
 * @}
 */
} // namespace sptk
