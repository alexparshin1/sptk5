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

#include <sptk5/net/ServerConnection.h>
#include <sptk5/threads/Runable.h>

namespace sptk {

/**
 * @addtogroup net Networking Classes
 * @{
 */

class SP_EXPORT RunableServerConnection
    : public ServerConnection
    , public Runable

{
public:
    /**
     * Constructor
     * @param server            Server that created this connection
     * @param type              Connection type
     * @param connectionAddress Connection address
     * @param connectionFunction Connection function executed for each new client connection to server
     */
    RunableServerConnection(TCPServer& server, Type type, const sockaddr_in* connectionAddress,
                             Function connectionFunction = {})
        : ServerConnection(server, type, connectionAddress)
        , Runable("RunnableServerConnection")
        , m_connectionFunction(std::move(connectionFunction))
    {
    }

protected:
    void run() override
    {
        if (m_connectionFunction)
        {
            m_connectionFunction(*this);
        }
    }

private:
    Function m_connectionFunction; ///< Function that is executed for each client connection
};

/**
 * @}
 */
} // namespace sptk
