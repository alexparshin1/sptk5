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

#ifndef __TCP_SERVER_LISTENER_H__
#define __TCP_SERVER_LISTENER_H__

#include <sptk5/net/ServerConnection.h>
#include <sptk5/Logger.h>
#include <set>
#include <iostream>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk
{

class TCPServer;

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * @brief Internal TCP server listener thread
 */
class TCPServerListener: public Thread, public std::mutex
{
    /**
     * TCP server created connection
     */
    TCPServer*      m_server;

    /**
     * Listener socket
     */
    TCPSocket       m_listenerSocket;

    /**
     * Last socket error
     */
    String          m_error;

public:
    /**
     * @brief Constructor
     * @param server CTCPServer*, TCP server created connection
     * @param port int, Listener port number
     */
    TCPServerListener(TCPServer* server, uint16_t port);

    /**
     * @brief Thread function
     */
    virtual void threadFunction();

    /**
     * @brief Custom thread terminate method
     */
    virtual void terminate();

    /**
     * @brief Start socket listening
     */
    void listen()
    {
        m_listenerSocket.listen();
    }

    /**
     * @brief Returns listener port number
     */
    uint16_t port() const
    {
        return m_listenerSocket.host().port();
    }

    /**
     * @brief Returns latest socket error (if any)
     */
    String error() const
    {
        return m_error;
    }
};

/**
 * @}
 */
}
#endif
