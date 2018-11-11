/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPServer.h - description                              ║
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

#ifndef __CTCPSERVER_H__
#define __CTCPSERVER_H__

#include <sptk5/net/ServerConnection.h>
#include <sptk5/Logger.h>
#include <set>
#include <iostream>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk
{

class TCPServerListener;

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * @brief TCP server
 *
 * For every incoming connection, creates connection thread.
 */
class TCPServer : protected Thread
{
    friend class TCPServerListener;
    friend class ServerConnection;

    /**
     * Mutex protecting internal data
     */
    mutable SharedMutex                     m_mutex;

    /**
     * Server listener object
     */
    TCPServerListener*                      m_listenerThread;

    /**
     * Optional logger
     */
    Logger*                                 m_logger;

    /**
     * Per-connection thread set
     */
    std::set<ServerConnection*>             m_connectionThreads;

    /**
     * Per-connection thread set
     */
    SynchronizedQueue<ServerConnection*>    m_completedConnectionThreads;

    /**
     * Lock to protect per-connection thread set manipulations
     */
    mutable SharedMutex                     m_connectionThreadsLock;

protected:
    /**
     * @brief Screens incoming connection request
     *
     * Method is called right after connection request is accepted,
     * and allows ignoring unwanted connections. By default simply returns true (allow).
     * @param connectionRequest Incoming connection information
     */
    virtual bool allowConnection(sockaddr_in* connectionRequest);

    /**
     * @brief Creates connection thread derived from CTCPServerConnection or CSSLServerConnection
     *
     * Application should override this method to create concrete connection object.
     * Created connection object is maintained by CTCPServer.
     * @param connectionSocket  Already accepted incoming connection socket
     * @param peer              Incoming connection information
     */
    virtual ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) = 0;

    /**
     * @brief Receives notification on connection thread created
     * @param connection        Newly created connection thread
     */
    void registerConnection(ServerConnection* connection);

    /**
     * @brief Receives notification on connection thread exited
     *
     * Connection thread is self-destructing immediately after exiting this method
     * @param connection        Exited connection thread
     */
    void unregisterConnection(ServerConnection* connection);

    /**
     * Server thread function.
     * Cleans up completed connections.
     */
    void threadFunction() override;

    /*
     * Custom terminate function.
     */
    void terminate() override;

public:
    /**
     * @brief Constructor
     */
    TCPServer(Logger* logger=NULL);

    /**
     * @brief Destructor
     */
    ~TCPServer() override;

    /**
     * @brief Returns listener port number
     */
    uint16_t port() const;

    /**
     * @brief Starts listener
     * @param port              Listener port number
     */
    void listen(uint16_t port);

    /**
     * @brief Stops listener
     */
    void stop();

    /**
     * @brief Returns server state
     */
    bool active() const
    {
        return m_listenerThread != NULL;
    }

    /**
     * @brief Server operation log
     */
    void log(LogPriority priority, std::string message)
    {
        if (m_logger)
            m_logger->log(priority, message);
    }
};

/**
 * @}
 */
}
#endif
