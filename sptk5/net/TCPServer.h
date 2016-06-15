/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TCPServer.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

namespace sptk
{

class TCPServer;

/// @addtogroup net Networking Classes
/// @{

/// @brief Internal TCP server listener thread
class TCPServerListener: public Thread
{
    TCPServer*     m_server;           ///< TCP server created connection
    TCPSocket      m_listenerSocket;   ///< Listener socket
    std::string     m_error;            ///< Last socket error
public:
    /// @brief Constructor
    /// @param server CTCPServer*, TCP server created connection
    /// @param port int, Listener port number
    TCPServerListener(TCPServer* server, int port);

    /// @brief Thread function
    virtual void threadFunction();

    /// @brief Custom thread terminate method
    virtual void terminate();

    /// @brief Start socket listening
    void listen()
    {
        m_listenerSocket.listen();
    }

    /// @brief Returns listener port number
    int port() const
    {
        return m_listenerSocket.port();
    }

    /// @brief Returns latest socket error (if any)
    std::string error() const
    {
        return m_error;
    }
};

/// @brief TCP server
///
/// For every incoming connection, creates connection thread.
class TCPServer: public Synchronized
{
    friend class TCPServerListener;
    friend class ServerConnection;
    TCPServerListener*             m_listenerThread;           ///< Server listener object
    Logger*                        m_logger;                   ///< Optional logger
    std::set<ServerConnection*>    m_connectionThreads;        ///< Per-connection thread set
    Synchronized                   m_connectionThreadsLock;    ///< Lock to protect per-connection thread set manipulations
protected:
    /// @brief Screens incoming connection request
    ///
    /// Method is called right after connection request is accepted,
    /// and allows ignore unwanted connections. By default simply returns true.
    /// @param connectionRequest sockaddr_in*, Incoming connection information
    virtual bool allowConnection(sockaddr_in* connectionRequest);

    /// @brief Creates connection thread derived from CTCPServerConnection or CSSLServerConnection
    ///
    /// Application should override this method to create concrete connection object.
    /// Created connection object is maintained by CTCPServer.
    /// @param connectionSocket SOCKET, Already accepted incoming connection socket
    /// @param peer sockaddr_in*, Incoming connection information
    virtual ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) = 0;

    /// @brief Receives notification on connection thread created
    /// @param connection CServerConnection*, Newly created connection thread
    void registerConnection(ServerConnection* connection);

    /// @brief Receives notification on connection thread exited
    ///
    /// Connection thread is self-destructing immediately after exiting this method
    /// @param connection CServerConnection*, Exited connection thread
    void unregisterConnection(ServerConnection* connection);

public:
    /// @brief Constructor
    TCPServer(Logger* logger=NULL)
    : m_listenerThread(NULL), m_logger(logger)
    {
    }

    /// @brief Destructor
    virtual ~TCPServer()
    {
        stop();
    }

    /// @brief Returns listener port number
    int port() const
    {
        if (!m_listenerThread)
            return 0;
        return m_listenerThread->port();
    }

    /// @brief Starts listener
    /// @param port int, Listener port number
    void listen(int port);

    /// @brief Stops listener
    void stop();

    /// @brief Returns server state
    bool active() const
    {
        return m_listenerThread != NULL;
    }

    /// @brief Server operation log
    void log(LogPriority priority, std::string message)
    {
        if (m_logger)
            *m_logger << priority << message << std::endl;
    }
};

/// @}
}
#endif
