/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ServerConnection.h - description                       ║
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

#ifndef __SERVERCONNECTION_H__
#define __SERVERCONNECTION_H__

#include <sptk5/net/TCPSocket.h>
#include <sptk5/threads/Thread.h>

namespace sptk
{

class TCPServer;

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * @brief Abstract TCP or SSL server connection thread
 *
 * Used a base class for CTCPServerConnection and COpenSSLServerConnection
 */
class ServerConnection: public Thread
{
    friend class TCPServer;
protected:
    /**
     * Connection socket
     */
    TCPSocket*     m_socket;

    /**
     * Parent server object
     */
    TCPServer*     m_server;

public:
    /**
     * @brief Constructor
     * @param connectionSocket SOCKET, Already accepted by accept() function incoming connection socket
     * @param threadName std::string, Already accepted by accept() function incoming connection socket
     */
    ServerConnection(SOCKET connectionSocket, std::string threadName)
    : Thread(threadName), m_socket(nullptr), m_server(nullptr)
    {
    }

    /**
     * @brief Destructor
     */
    virtual ~ServerConnection()
    {
        if (m_socket)
            delete m_socket;
    }

    /**
     * @brief Thread function
     */
    virtual void threadFunction() = 0;

    /**
     * @brief Method that is called upon thread exit
     */
    virtual void onThreadExit();
};

/**
 * @}
 */
}
#endif
