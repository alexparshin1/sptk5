/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ServerConnection.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/threads/Runable.h>

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
class ServerConnection: public Runable
{
    friend class SMQServer;
    friend class TCPServer;

    /**
     * Parent server object
     */
    TCPServer&     m_server;

    /**
     * Connection socket
     */
    TCPSocket*     m_socket;

protected:

    TCPSocket& socket() const;

    void setSocket(TCPSocket* socket);

    TCPServer& server() const;

public:

    /**
     * @brief Constructor
     * @param connectionSocket  Already accepted by accept() function incoming connection socket
     * @param taskName          Task name
     */
    ServerConnection(TCPServer& server, SOCKET connectionSocket, const String& taskName);

    /**
     * @brief Destructor
     */
    ~ServerConnection() override;
};

/**
 * @}
 */
}
#endif
