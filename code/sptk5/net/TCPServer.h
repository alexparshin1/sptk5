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

#pragma once

#include <bitset>
#include <iostream>
#include <set>
#include <sptk5/Logger.h>
#include <sptk5/net/SSLKeys.h>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/threads/ThreadPool.h>

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
        THREAD_POOLING,
        MAX_MESSAGE_DETAIL
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
    explicit LogDetails(const MessageDetails& details)
        : m_details(details)
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
    explicit LogDetails(std::initializer_list<MessageDetail> details)
    {
        for (auto detail: details)
        {
            m_details.insert(detail);
        }
    }

    String toString(const String& delimiter = ",") const;

    /**
     * Query log details
     * @param detail            Log detail
     * @return true if log detail is set
     */
    bool has(MessageDetail detail) const
    {
        return m_details.find(detail) != m_details.end();
    }

    bool empty() const
    {
        return m_details.empty();
    }

private:
    MessageDetails m_details; ///< Log details set
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
     */
    TCPServer(const String& listenerName, ServerConnection::Type connectionType, size_t threadLimit = 16, LogEngine* logEngine = nullptr,
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
     * Starts listener
     * @param port              Listener port number
     */
    void listen(uint16_t port);

    /**
     * Stops listener
     */
    void stop() override;

    /**
     * Returns server state
     */
    bool active() const
    {
        return m_listenerThread != nullptr;
    }

    /**
     * Server operation log
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

    const LogDetails& logDetails() const
    {
        return m_logDetails;
    }

    /**
     * Set SSL keys for SSL connections (encrypted mode only)
     * @param sslKeys            SSL keys info
     */
    void setSSLKeys(std::shared_ptr<SSLKeys> sslKeys);

    /**
     * Get SSL keys for SSL connections (encrypted mode only)
     * @return SSL keys info
     */
    const SSLKeys& getSSLKeys() const;

    /**
     * Creates connection thread derived from TCPServerConnection or SSLServerConnection
     *
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param function          User-defined function that is called upon client connection to server
     */
    virtual void onConnection(const ServerConnection::Function& function);

protected:
    /**
     * Modify server host.
     * If listener is already active, don't modify exiting server host.
     * @param host              Server host
     */
    void host(const Host& host);

    /**
     * Screens incoming connection request
     *
     * Method is called right after connection request is accepted,
     * and allows ignoring unwanted connections. By default simply returns true (allow).
     * @param connectionRequest Incoming connection information
     */
    virtual bool allowConnection(sockaddr_in* connectionRequest);

    /**
     * Creates connection thread derived from TCPServerConnection or SSLServerConnection
     *
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param connectionSocket  Already accepted incoming connection socket
     * @param peer              Incoming connection information
     */
    virtual SServerConnection createConnection(SOCKET connectionSocket, const sockaddr_in* peer);

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
    mutable SharedMutex m_mutex;                         ///< Mutex protecting internal data
    std::shared_ptr<TCPServerListener> m_listenerThread; ///< Server listener
    std::shared_ptr<Logger> m_logger;                    ///< Optional logger
    std::shared_ptr<SSLKeys> m_sslKeys;                  ///< Optional SSL keys. Only used for SSL server.
    Host m_host;                                         ///< This host
    LogDetails m_logDetails;                             ///< Log details
    ServerConnection::Type m_connectionType;             ///< Connection type (TCP or SSL)
    ServerConnection::Function m_connectionFunction;     ///< User-defined function that is called upon client connection to server
};

/**
 * @}
 */
} // namespace sptk
