/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cnet>
#include <sptk5/wsdl/WSRequest.h>
#include <sptk5/net/RequestInfo.h>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// Abstract base class for different protocols used in Web Service servers
class WSProtocol
{
public:

    /// Constructor
    /// Note: the socket is not owned by this class and not discarded by destructor.
    /// @param socket           Connection socket
    /// @param headers          Connection HTTP headers
    WSProtocol(TCPSocket* socket, const HttpHeaders& headers)
    : m_socket(socket), m_headers(headers)
    {
    }

    WSProtocol(const WSProtocol&) = delete;

    /// Destructor
    ///
    /// Closes connection
    virtual ~WSProtocol()
    {
        if (m_socket)
            m_socket->close();
    }

    WSProtocol& operator = (const WSProtocol&) = delete;

    /// Process virtual method - to be implemented in derived classes
    virtual RequestInfo process() = 0;

protected:
    /**
     * Connection socket
     * @return Connection socket
     */
    TCPSocket& socket() { return *m_socket; }

    /**
     * Connection HTTP headers
     * @return Connection HTTP headers
     */
    HttpHeaders& headers() { return m_headers; }

    /**
     * Connection HTTP headers
     * @return Connection HTTP headers
     */
    String header(const sptk::String& name);

private:

    TCPSocket*      m_socket;   ///< Connection socket
    HttpHeaders     m_headers;  ///< Connection HTTP headers
};

/// @}

}

