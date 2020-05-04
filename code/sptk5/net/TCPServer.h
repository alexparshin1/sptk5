/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPServer.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <sptk5/net/ServerConnection.h>
#include <sptk5/Logger.h>
#include <set>
#include <iostream>
#include <sptk5/threads/SynchronizedQueue.h>
#include <sptk5/threads/ThreadPool.h>
#include <sptk5/net/SSLKeys.h>

namespace sptk
{

class TCPServerListener;

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * TCP server
 *
 * For every incoming connection, creates connection thread.
 */
class SP_EXPORT TCPServer : public ThreadPool
{
    friend class TCPServerListener;
    friend class ServerConnection;

    mutable SharedMutex                     m_mutex;            ///< Mutex protecting internal data
    std::shared_ptr<TCPServerListener>      m_listenerThread;   ///< Server listener
    std::shared_ptr<Logger>                 m_logger;           ///< Optional logger
    std::shared_ptr<SSLKeys>                m_sslKeys;          ///< Optional SSL keys. Only used for SSL server.
    String                                  m_hostname;         ///< This host name

protected:
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
    virtual ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) = 0;

    /**
     * Thread event callback function
     *
     * Receives events that occur in the threads
     * @param thread            Thread where event occured
     * @param eventType         Thread event type
     * @param runable           Related runable (if any)
     */
    void threadEvent(Thread* thread, ThreadEvent::Type eventType, Runable* runable) override;

public:
    /**
     * Constructor
     * @param listenerName      Logical name of the listener
     * @param threadLimit       Number of worker threads in thread pool
     * @param logEngine         Optional log engine
     */
    explicit TCPServer(const String& listenerName, size_t threadLimit=16, LogEngine* logEngine=nullptr);

    /**
     * Destructor
     */
    ~TCPServer() override;

    /**
     * Get current hostname of the server
     * @return
     */
    virtual String hostname() const;

    /**
     * Returns listener port number
     */
    uint16_t port() const;

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
    void log(LogPriority priority, const String& message)
    {
        if (m_logger)
            m_logger->log(priority, message);
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
};

/**
 * @}
 */
}
#endif
